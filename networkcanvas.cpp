#include "networkcanvas.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPolygonF>

#include <algorithm>
#include <cmath>

namespace {

const QColor kDeleteColor(230, 80, 0);   // 删除模式下的高亮色
constexpr double kEdgeHitTolerance = 6.0; // 点到线段距离 ≤ 6px 算点中

QColor kindColor(NeuronKind k)
{
    switch (k) {
    case NeuronKind::Input:  return QColor(255, 0, 0);   // 配色沿用原程序：输入红、隐层绿、输出蓝
    case NeuronKind::Hidden: return QColor(0, 255, 0);
    case NeuronKind::Output: return QColor(0, 0, 255);
    }
    return Qt::black;
}

QString num(double v)
{
    return QString::number(v, 'g', 6);
}

// 两个圆之间的那段线：沿圆心连线，从 a 的圆周画到 b 的圆周。
// 原来固定从“右边缘”画到“左边缘”，目标在左边时线会横穿两个圆。
QLineF segmentBetween(const Neuron &a, const Neuron &b)
{
    const QPointF d = b.pos - a.pos;
    const double len = std::hypot(d.x(), d.y());
    const double ra = NetworkCanvas::radius(a.kind);
    const double rb = NetworkCanvas::radius(b.kind);
    if (len <= ra + rb + 1.0)   // 两个圆叠在一起时没法“圆周到圆周”，直接连圆心
        return QLineF(a.pos, b.pos);
    const QPointF u = d / len;
    return QLineF(a.pos + u * ra, b.pos - u * rb);
}

// 点到线段的距离：投影参数 t 夹在 [0,1]，再求点到投影点的距离。
// 原来用“竖直方向的距离 < 3”且要求 x1 < x2：从右往左的线、竖直的线都点不中，竖直线还会除以 0。
double distanceToSegment(const QPointF &p, const QLineF &s)
{
    const QPointF d = s.p2() - s.p1();
    const double len2 = QPointF::dotProduct(d, d);
    if (len2 == 0.0)
        return QLineF(p, s.p1()).length();
    const double t = std::clamp(QPointF::dotProduct(p - s.p1(), d) / len2, 0.0, 1.0);
    return QLineF(p, s.p1() + t * d).length();
}

void drawArrowHead(QPainter &p, const QLineF &line, const QColor &c)
{
    if (line.length() < 1.0)
        return;
    const double a = std::atan2(line.dy(), line.dx());
    const double len = 12.0, spread = 0.45;
    const QPointF tip = line.p2();
    const QPointF l = tip - QPointF(std::cos(a - spread), std::sin(a - spread)) * len;
    const QPointF r = tip - QPointF(std::cos(a + spread), std::sin(a + spread)) * len;
    p.setPen(Qt::NoPen);
    p.setBrush(c);
    p.drawPolygon(QPolygonF{tip, l, r});
}

} // namespace

NetworkCanvas::NetworkCanvas(Network *net, QWidget *parent)
    : QWidget(parent), m_net(net)
{
    setMouseTracking(true);   // 删除模式下要跟踪鼠标悬停，给出高亮
    setMinimumSize(600, 400);
}

double NetworkCanvas::radius(NeuronKind kind)
{
    return kind == NeuronKind::Hidden ? 60.0 : 50.0;
}

std::unordered_map<int, const Neuron *> NetworkCanvas::byId() const
{
    std::unordered_map<int, const Neuron *> m;
    m.reserve(m_net->neurons().size());
    for (const Neuron &n : m_net->neurons())
        m[n.id] = &n;
    return m;
}

// ======================= 模式 =======================

void NetworkCanvas::setMode(Mode m)
{
    if (m_mode == m)
        return;
    m_mode = m;
    m_hoverNeuron = m_hoverEdge = -1;
    if (m == Mode::Normal)
        unsetCursor();
    else
        setCursor(Qt::CrossCursor);
    if (m == Mode::DeleteNeuron)
        emit message(tr("删除点：点击要删除的神经元（Esc 或右键取消）"));
    else if (m == Mode::DeleteEdge)
        emit message(tr("删除线：点击要删除的线或它的权值标签（Esc 或右键取消）"));
    emit modeChanged(m);
    update();
}

void NetworkCanvas::cancelInteraction()
{
    const bool busy = m_mode != Mode::Normal || m_linkFrom >= 0;
    m_linkFrom = -1;
    m_dragId = -1;
    setMode(Mode::Normal);
    if (busy)
        emit message(tr("已取消"));
    update();
}

// ======================= 命中判断 =======================

int NetworkCanvas::neuronAt(const QPointF &p) const
{
    const auto &ns = m_net->neurons();
    // 倒序：后画的在上面，叠在一起时只取最上面那一个（原来每一层各取一个，叠住的会一起被拖走）
    for (auto it = ns.rbegin(); it != ns.rend(); ++it) {
        const double r = radius(it->kind);
        if (QPointF::dotProduct(p - it->pos, p - it->pos) <= r * r)   // 真正的圆，而不是外接正方形
            return it->id;
    }
    return -1;
}

QLineF NetworkCanvas::edgeLine(const Edge &e) const
{
    const Neuron *a = m_net->neuron(e.from);
    const Neuron *b = m_net->neuron(e.to);
    return (a && b) ? segmentBetween(*a, *b) : QLineF();
}

QRectF NetworkCanvas::weightLabelRect(const Edge &e, const QLineF &line) const
{
    const QString text = QStringLiteral("weight:%1").arg(num(e.weight));
    const QFontMetricsF fm(font());
    QRectF r(0, 0, fm.horizontalAdvance(text) + 8, fm.height() + 2);
    r.moveCenter(line.center());
    return r;
}

int NetworkCanvas::edgeAt(const QPointF &p) const
{
    const auto ids = byId();
    int best = -1;
    double bestDist = kEdgeHitTolerance;
    for (const Edge &e : m_net->edges()) {
        auto a = ids.find(e.from), b = ids.find(e.to);
        if (a == ids.end() || b == ids.end())
            continue;
        const QLineF line = segmentBetween(*a->second, *b->second);
        if (weightLabelRect(e, line).contains(p))   // 点权值标签也算点中这条线
            return e.id;
        const double d = distanceToSegment(p, line);
        if (d <= bestDist) {
            bestDist = d;
            best = e.id;
        }
    }
    return best;
}

QPointF NetworkCanvas::suggestPosition(NeuronKind kind) const
{
    // 原来新神经元永远出现在同一个点，几个摞在一起要一个个拖开。
    // 现在在本层那一列里从上往下找第一个空位，列满了再往右挪一点继续找。
    const double r = radius(kind);
    const double x0 = kind == NeuronKind::Input ? 100 : kind == NeuronKind::Hidden ? 300 : 500;
    const double bottom = std::max<double>(height(), 600) - r;
    for (double dx = 0; dx <= 120; dx += 40) {
        for (double y = 100; y <= bottom; y += 2 * r + 20) {
            const QPointF c(x0 + dx, y);
            bool free = true;
            for (const Neuron &n : m_net->neurons()) {
                if (QLineF(n.pos, c).length() < r + radius(n.kind) + 10) {
                    free = false;
                    break;
                }
            }
            if (free)
                return c;
        }
    }
    return QPointF(x0, 100);
}

QPointF NetworkCanvas::clampToCanvas(NeuronKind kind, const QPointF &p) const
{
    const double r = radius(kind);
    const double maxX = std::max(r, width() - r);
    const double maxY = std::max(r, height() - r);
    return QPointF(std::clamp(p.x(), r, maxX), std::clamp(p.y(), r, maxY));
}

// ======================= 绘制 =======================

void NetworkCanvas::paintEvent(QPaintEvent *)
{
    // 注意：这里不调用 update()。原来 paintEvent 里调 update()，画完又排下一次重绘，空闲时也占满一个 CPU 核。
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), palette().window());

    const auto ids = byId();

    // 1) 先画线后画神经元，线不会压住神经元上的字
    for (const Edge &e : m_net->edges()) {
        auto a = ids.find(e.from), b = ids.find(e.to);
        if (a == ids.end() || b == ids.end())
            continue;
        const QLineF line = segmentBetween(*a->second, *b->second);
        const bool hot = (m_mode == Mode::DeleteEdge && e.id == m_hoverEdge);
        const QColor c = hot ? kDeleteColor : QColor(Qt::black);
        p.setPen(QPen(c, hot ? 4 : 2));
        p.drawLine(line);
        drawArrowHead(p, line, c);   // 箭头表示方向（原来看不出谁连向谁）

        const QRectF label = weightLabelRect(e, line);
        p.setPen(Qt::NoPen);
        p.setBrush(palette().window());
        p.drawRoundedRect(label, 3, 3);
        p.setPen(c);
        p.drawText(label, Qt::AlignCenter, QStringLiteral("weight:%1").arg(num(e.weight)));
    }

    // 2) 正在拉的线：灰色虚线从起点的圆周跟到鼠标（鼠标还在圆里时不画）
    if (m_linkFrom >= 0) {
        auto a = ids.find(m_linkFrom);
        if (a != ids.end()) {
            const QPointF c = a->second->pos;
            const QPointF d = m_linkPos - c;
            const double len = std::hypot(d.x(), d.y());
            const double r = radius(a->second->kind);
            if (len > r) {
                p.setPen(QPen(Qt::gray, 2, Qt::DashLine));
                p.drawLine(c + d / len * r, m_linkPos);
            }
        }
    }

    // 3) 神经元
    for (const Neuron &n : m_net->neurons())
        drawNeuron(p, n, m_mode == Mode::DeleteNeuron && n.id == m_hoverNeuron);
}

void NetworkCanvas::drawNeuron(QPainter &p, const Neuron &n, bool hot) const
{
    const double r = radius(n.kind);
    const QColor c = kindColor(n.kind);
    QColor fill = hot ? kDeleteColor : c;
    fill.setAlpha(hot ? 45 : 18);
    p.setPen(QPen(hot ? kDeleteColor : c, hot ? 5 : 3));
    p.setBrush(fill);
    p.drawEllipse(n.pos, r, r);

    QStringList lines;
    if (n.kind == NeuronKind::Hidden) {
        lines << tr("阈值:%1").arg(num(n.threshold))
              << tr("激活函数:%1").arg(n.activation == Activation::Linear ? tr("线性") : tr("阶跃"));
    }
    lines << tr("总输入:%1").arg(num(n.input))
          << QStringLiteral("Key:%1").arg(n.id)
          << tr("总输出:%1").arg(num(n.output));

    const QFontMetricsF fm(font());
    const double lh = fm.height() + 2;
    double y = n.pos.y() - lh * lines.size() / 2.0;
    p.setPen(hot ? kDeleteColor : c.darker(140));   // 字比圆圈深一点，浅绿底上更好认
    for (const QString &s : lines) {
        p.drawText(QRectF(n.pos.x() - r, y, 2 * r, lh), Qt::AlignCenter, s);
        y += lh;
    }
}

// ======================= 鼠标 =======================

void NetworkCanvas::mousePressEvent(QMouseEvent *event)
{
    const QPointF pos = event->position();

    if (event->button() == Qt::RightButton) {
        if (m_mode != Mode::Normal) {   // 删除模式下右键 = 取消
            cancelInteraction();
            return;
        }
        const int id = neuronAt(pos);
        if (id < 0)
            return;   // 空白处按下不开始连线（原来会沿用上一次的起点，凭空连出一条线）
        if (m_net->neuron(id)->kind == NeuronKind::Output) {
            emit message(Network::errorText(ConnectError::FromOutput));
            return;
        }
        m_linkFrom = id;
        m_linkPos = pos;
        update();
        return;
    }

    if (event->button() != Qt::LeftButton)
        return;

    if (m_mode == Mode::DeleteNeuron) {
        const int id = neuronAt(pos);
        if (id < 0) {
            emit message(tr("这里没有神经元。点要删除的神经元，Esc 退出删除模式"));
            return;
        }
        m_net->removeNeuron(id);   // 连带删除相连的线，在模型里一处完成
        setMode(Mode::Normal);
        emit message(tr("已删除神经元 %1 及其连线").arg(id));
        emit networkEdited();
        update();
        return;
    }
    if (m_mode == Mode::DeleteEdge) {
        const int id = edgeAt(pos);
        if (id < 0) {
            emit message(tr("这里没有线。点要删除的线，Esc 退出删除模式"));
            return;
        }
        m_net->removeEdge(id);
        setMode(Mode::Normal);
        emit message(tr("已删除线 %1").arg(id));
        emit networkEdited();
        update();
        return;
    }

    const int id = neuronAt(pos);
    if (id >= 0) {
        m_dragId = id;
        m_dragOffset = pos - m_net->neuron(id)->pos;
    }
}

void NetworkCanvas::mouseMoveEvent(QMouseEvent *event)
{
    const QPointF pos = event->position();

    // 拖动：按下时已经记住了拖的是谁，这里不再判断“鼠标还在不在圆里”，鼠标甩得再快也跟得上
    if (m_dragId >= 0 && (event->buttons() & Qt::LeftButton)) {
        if (const Neuron *n = m_net->neuron(m_dragId)) {
            m_net->setPosition(m_dragId, clampToCanvas(n->kind, pos - m_dragOffset));
            update();
        }
        return;
    }
    // 连线：用 buttons()（当前按着哪些键）判断；原来写成 button()，移动事件里它恒为 NoButton
    if (m_linkFrom >= 0 && (event->buttons() & Qt::RightButton)) {
        m_linkPos = pos;
        update();
        return;
    }
    if (m_mode != Mode::Normal)
        updateHover(pos);
}

void NetworkCanvas::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragId = -1;   // 原来用 buttons()==LeftButton 判断松开，松开时左键已不在 buttons() 里，永远进不去
        return;
    }
    if (event->button() == Qt::RightButton && m_linkFrom >= 0) {
        const int from = m_linkFrom;
        m_linkFrom = -1;
        const int to = neuronAt(event->position());
        if (to >= 0) {
            ConnectError err = ConnectError::None;
            if (m_net->addEdge(from, to, &err) >= 0) {
                emit message(tr("已连线 %1 → %2，双击线可设权值").arg(from).arg(to));
                emit networkEdited();
            } else {
                emit message(Network::errorText(err));
            }
        }
        update();
    }
}

void NetworkCanvas::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton || m_mode != Mode::Normal)
        return;
    m_dragId = -1;
    const QPointF pos = event->position();
    const int id = neuronAt(pos);
    if (id >= 0) {
        emit neuronActivated(id);
        return;
    }
    const int edgeId = edgeAt(pos);
    if (edgeId >= 0)
        emit edgeActivated(edgeId);
}

void NetworkCanvas::leaveEvent(QEvent *)
{
    if (m_hoverNeuron >= 0 || m_hoverEdge >= 0) {
        m_hoverNeuron = m_hoverEdge = -1;
        update();
    }
}

void NetworkCanvas::updateHover(const QPointF &p)
{
    const int n = m_mode == Mode::DeleteNeuron ? neuronAt(p) : -1;
    const int e = m_mode == Mode::DeleteEdge ? edgeAt(p) : -1;
    if (n != m_hoverNeuron || e != m_hoverEdge) {   // 只有高亮对象变了才重绘
        m_hoverNeuron = n;
        m_hoverEdge = e;
        update();
    }
}
