// tst_ann.cpp —— 自动化测试：模型（Network）、画布交互（NetworkCanvas）、主窗口和对话框。
// 用例名后面的 Rx 对应《修改记录》里原版 bug 的编号，每条 bug 都有一个回归测试。
// 运行（命令行）：
//   set QT_QPA_PLATFORM=offscreen        不弹窗口
//   set QT_QPA_FONTDIR=C:\Windows\Fonts  offscreen 平台要自己指定字体目录，截图里才有字
//   set ANN_SHOTS=<目录>                 可选：顺便保存“修改后”的截图
//   tst_ann.exe

#include <QtTest>
#include <QtWidgets>

#include <functional>
#include <random>
#include <unordered_map>

#ifdef Q_OS_WIN
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#endif

#include "dialog.h"
#include "dialog2.h"
#include "dialog3.h"
#include "mainwindow.h"
#include "network.h"
#include "networkcanvas.h"

namespace {

// ---------------- 鼠标 ----------------
void mouse(QWidget *w, QEvent::Type t, QPointF p, Qt::MouseButton b, Qt::MouseButtons bs)
{
    QMouseEvent ev(t, p, w->mapToGlobal(p), b, bs, Qt::NoModifier);
    QApplication::sendEvent(w, &ev);
}
void leftClick(QWidget *w, QPointF p)
{
    mouse(w, QEvent::MouseButtonPress, p, Qt::LeftButton, Qt::LeftButton);
    mouse(w, QEvent::MouseButtonRelease, p, Qt::LeftButton, Qt::NoButton);
}
void leftDrag(QWidget *w, QPointF a, QPointF b, int steps)
{
    mouse(w, QEvent::MouseButtonPress, a, Qt::LeftButton, Qt::LeftButton);
    for (int i = 1; i <= steps; ++i)
        mouse(w, QEvent::MouseMove, a + (b - a) * i / steps, Qt::NoButton, Qt::LeftButton);
    mouse(w, QEvent::MouseButtonRelease, b, Qt::LeftButton, Qt::NoButton);
}
void rightDrag(QWidget *w, QPointF a, QPointF b)
{
    mouse(w, QEvent::MouseButtonPress, a, Qt::RightButton, Qt::RightButton);
    mouse(w, QEvent::MouseMove, b, Qt::NoButton, Qt::RightButton);
    mouse(w, QEvent::MouseButtonRelease, b, Qt::RightButton, Qt::NoButton);
}
void doubleClick(QWidget *w, QPointF p)
{
    leftClick(w, p);
    mouse(w, QEvent::MouseButtonDblClick, p, Qt::LeftButton, Qt::LeftButton);
    mouse(w, QEvent::MouseButtonRelease, p, Qt::LeftButton, Qt::NoButton);
}
void hover(QWidget *w, QPointF p)
{
    mouse(w, QEvent::MouseMove, p, Qt::NoButton, Qt::NoButton);
}

// ---------------- 对话框：预约“下一个弹出的模态窗口”怎么处理 ----------------
void onNextDialog(std::function<void(QDialog *)> fn)
{
    QTimer::singleShot(0, [fn]() {
        QDialog *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (!d) {   // 兜底：在顶层窗口里找正在显示的对话框
            const auto tops = QApplication::topLevelWidgets();
            for (QWidget *t : tops)
                if (auto *dd = qobject_cast<QDialog *>(t); dd && dd->isVisible())
                    d = dd;
        }
        if (d)
            fn(d);
        else
            qWarning("没有找到弹出的对话框");
    });
}
void fillAndOk(QDialog *d, const QString &text, const char *radio = nullptr)
{
    if (auto *le = d->findChild<QLineEdit *>())
        le->setText(text);
    if (radio)
        if (auto *rb = d->findChild<QRadioButton *>(radio))
            rb->setChecked(true);
    d->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click();
}
void pressCancel(QDialog *d)
{
    d->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Cancel)->click();
}

// ---------------- 杂项 ----------------
QString edgesOf(const Network &n)
{
    QStringList s;
    for (const Edge &e : n.edges())
        s << QStringLiteral("%1→%2").arg(e.from).arg(e.to);
    return QLatin1Char('[') + s.join(QStringLiteral(", ")) + QLatin1Char(']');
}
void saveShot(QWidget *w, const QString &name)
{
    const QString dir = qEnvironmentVariable("ANN_SHOTS");
    if (dir.isEmpty())
        return;
    QDir().mkpath(dir);
    w->grab().save(dir + QStringLiteral("/after_") + name + QStringLiteral(".png"));
}
qint64 cpuMs()
{
#ifdef Q_OS_WIN
    FILETIME c, e, k, u;
    GetProcessTimes(GetCurrentProcess(), &c, &e, &k, &u);
    auto ms = [](FILETIME f) {
        ULARGE_INTEGER x;
        x.LowPart = f.dwLowDateTime;
        x.HighPart = f.dwHighDateTime;
        return qint64(x.QuadPart / 10000);
    };
    return ms(k) + ms(u);
#else
    return -1;
#endif
}
class PaintCounter : public QObject
{
public:
    int n = 0;
    bool eventFilter(QObject *, QEvent *e) override
    {
        if (e->type() == QEvent::Paint)
            ++n;
        return false;
    }
};

// R4 用的网络：0→2, 1→3, 2→3（2 是隐层，3 同时收到输入层和隐层的线，即“跳层连接”）, 3→4
struct SkipNet
{
    Network net;
    int i0, i1, h2, h3, o4;
};
SkipNet buildSkipNet()
{
    SkipNet s;
    s.i0 = s.net.addNeuron(NeuronKind::Input, {90, 150});
    s.i1 = s.net.addNeuron(NeuronKind::Input, {90, 450});
    s.h2 = s.net.addNeuron(NeuronKind::Hidden, {300, 150});
    s.h3 = s.net.addNeuron(NeuronKind::Hidden, {500, 300});
    s.o4 = s.net.addNeuron(NeuronKind::Output, {700, 300});
    const int pairs[][2] = {{s.i0, s.h2}, {s.i1, s.h3}, {s.h2, s.h3}, {s.h3, s.o4}};
    for (const auto &p : pairs)
        s.net.setWeight(s.net.addEdge(p[0], p[1]), 0.4);
    s.net.setInputValue(s.i0, 1);
    s.net.setInputValue(s.i1, 1);
    return s;
}

} // namespace

// =====================================================================
//  模型
// =====================================================================
class NetworkTest : public QObject
{
    Q_OBJECT

private slots:
    void idsAreUniqueAndNeverReused()
    {
        Network n;
        QCOMPARE(n.addNeuron(NeuronKind::Input, {}), 0);
        QCOMPARE(n.addNeuron(NeuronKind::Hidden, {}), 1);
        QCOMPARE(n.addNeuron(NeuronKind::Output, {}), 2);
        QVERIFY(n.removeNeuron(1));
        QVERIFY(!n.removeNeuron(1));                       // 已经删过了
        QCOMPARE(n.addNeuron(NeuronKind::Hidden, {}), 3);  // 编号不复用
        QCOMPARE(n.addEdge(0, 3), 0);
        QCOMPARE(n.addEdge(3, 2), 1);
        QVERIFY(n.removeEdge(0));
        QCOMPARE(n.addEdge(0, 3), 2);                      // 边的编号也不复用
        n.clear();
        QCOMPARE(n.addNeuron(NeuronKind::Input, {}), 0);   // 清空后两个计数器都归零
        n.addNeuron(NeuronKind::Output, {});
        QCOMPARE(n.addEdge(0, 1), 0);
    }

    // R1 + R2：删神经元只删它自己的线，一条不漏、一条不多
    void removeNeuronRemovesExactlyItsEdges()
    {
        Network n;
        const int i0 = n.addNeuron(NeuronKind::Input, {});
        const int i1 = n.addNeuron(NeuronKind::Input, {});
        const int h = n.addNeuron(NeuronKind::Hidden, {});
        const int o1 = n.addNeuron(NeuronKind::Output, {});
        const int o2 = n.addNeuron(NeuronKind::Output, {});
        n.addEdge(h, o1);   // 隐层的两条出线在边表里相邻：原来的 erase 循环会漏删第二条
        n.addEdge(h, o2);
        n.addEdge(i0, h);
        n.addEdge(i1, h);
        n.addEdge(i0, o1);
        QVERIFY(n.removeNeuron(h));
        QCOMPARE(edgesOf(n), QStringLiteral("[0→3]"));
        QVERIFY(n.removeNeuron(i0));
        QCOMPARE(edgesOf(n), QStringLiteral("[]"));
        QVERIFY(n.neuron(i1));
        QVERIFY(n.neuron(o2));
    }

    // R3：删过线之后再连线，设的权值照样参与计算
    void weightWorksAfterDeletingAnEdge()
    {
        Network n;
        const int i = n.addNeuron(NeuronKind::Input, {});
        const int o = n.addNeuron(NeuronKind::Output, {});
        QVERIFY(n.removeEdge(n.addEdge(i, o)));
        const int e = n.addEdge(i, o);
        QVERIFY(e >= 0);
        QVERIFY(n.setInputValue(i, 1));
        QVERIFY(n.setWeight(e, 2));
        QVERIFY(n.run().ok);
        QCOMPARE(n.neuron(o)->output, 2.0);
    }

    // R3b：清空后重新搭同样的网络，结果一样
    void clearThenRebuildGivesSameResult()
    {
        Network n;
        auto build = [&n]() {
            const int i = n.addNeuron(NeuronKind::Input, {});
            const int o = n.addNeuron(NeuronKind::Output, {});
            n.setWeight(n.addEdge(i, o), 2);
            n.setInputValue(i, 1);
            n.run();
            return n.neuron(o)->output;
        };
        QCOMPARE(build(), 2.0);
        n.clear();
        QCOMPARE(build(), 2.0);
    }

    // R4：有跳层连接时也按完整的输入计算
    void skipConnectionComputedCorrectly()
    {
        SkipNet s = buildSkipNet();
        const RunResult r = s.net.run();
        QVERIFY(r.ok);
        QCOMPARE(s.net.neuron(s.h2)->output, 0.4);
        QCOMPARE(s.net.neuron(s.h3)->output, 0.56);    // 0.4×1 + 0.4×0.4
        QCOMPARE(s.net.neuron(s.o4)->output, 0.224);   // 0.56×0.4（原版得 0.16）
    }

    void runTwiceDoesNotAccumulate()
    {
        SkipNet s = buildSkipNet();
        s.net.run();
        s.net.run();
        QCOMPARE(s.net.neuron(s.o4)->output, 0.224);
    }

    // 断开以后再运行，不残留上一次的结果
    void runResetsStaleResults()
    {
        SkipNet s = buildSkipNet();
        s.net.run();
        for (const Edge &e : s.net.edges())
            if (e.to == s.o4) {
                s.net.removeEdge(e.id);
                break;
            }
        s.net.run();
        QCOMPARE(s.net.neuron(s.o4)->output, 0.0);
    }

    // R9 等：哪些线不让连
    void connectRules_data()
    {
        QTest::addColumn<int>("from");
        QTest::addColumn<int>("to");
        QTest::addColumn<int>("expected");
        // 0 输入、1 隐层、2 隐层、3 输出；已有 0→1、1→2
        QTest::newRow("self-loop 1->1") << 1 << 1 << int(ConnectError::SelfLoop);
        QTest::newRow("duplicate 0->1") << 0 << 1 << int(ConnectError::Duplicate);
        QTest::newRow("from output 3->1") << 3 << 1 << int(ConnectError::FromOutput);
        QTest::newRow("to input 1->0") << 1 << 0 << int(ConnectError::ToInput);
        QTest::newRow("cycle 2->1") << 2 << 1 << int(ConnectError::Cycle);
        QTest::newRow("no such neuron 99->1") << 99 << 1 << int(ConnectError::NoSuchNeuron);
        QTest::newRow("ok 0->3") << 0 << 3 << int(ConnectError::None);
        QTest::newRow("ok skip 0->2") << 0 << 2 << int(ConnectError::None);
    }
    void connectRules()
    {
        QFETCH(int, from);
        QFETCH(int, to);
        QFETCH(int, expected);
        Network n;
        n.addNeuron(NeuronKind::Input, {});
        n.addNeuron(NeuronKind::Hidden, {});
        n.addNeuron(NeuronKind::Hidden, {});
        n.addNeuron(NeuronKind::Output, {});
        QVERIFY(n.addEdge(0, 1) >= 0);
        QVERIFY(n.addEdge(1, 2) >= 0);
        const size_t before = n.edges().size();
        ConnectError err = ConnectError::Duplicate;   // 先放个别的值，确认函数确实写回了
        const int id = n.addEdge(from, to, &err);
        QCOMPARE(int(err), expected);
        if (expected == int(ConnectError::None)) {
            QVERIFY(id >= 0);
            QCOMPARE(n.edges().size(), before + 1);
        } else {
            QCOMPARE(id, -1);
            QCOMPARE(n.edges().size(), before);
        }
    }

    // 激活函数 + 阈值：输出 = f(总输入 − 阈值)
    void activationFunctions_data()
    {
        QTest::addColumn<int>("f");
        QTest::addColumn<double>("in");
        QTest::addColumn<double>("thr");
        QTest::addColumn<double>("out");
        QTest::newRow("linear in range") << 1 << 0.3 << 0.1 << 0.2;
        QTest::newRow("linear clip high") << 1 << 2.0 << 0.5 << 1.0;   // 原公式 f(x)−θ 得 0.5
        QTest::newRow("linear clip low") << 1 << -3.0 << 0.0 << -1.0;
        QTest::newRow("step above") << 2 << 0.6 << 0.5 << 1.0;         // 原公式得 0.5
        QTest::newRow("step equal") << 2 << 0.5 << 0.5 << 0.0;
        QTest::newRow("step below") << 2 << 0.2 << 0.5 << 0.0;         // 原公式得 0.5
    }
    void activationFunctions()
    {
        QFETCH(int, f);
        QFETCH(double, in);
        QFETCH(double, thr);
        QFETCH(double, out);
        Network n;
        const int i = n.addNeuron(NeuronKind::Input, {});
        const int h = n.addNeuron(NeuronKind::Hidden, {});
        n.setWeight(n.addEdge(i, h), 1.0);
        n.setInputValue(i, in);
        n.setHiddenParams(h, thr, Activation(f));
        QVERIFY(n.run().ok);
        QCOMPARE(n.neuron(h)->output, out);
    }

    // R6：新建的隐层神经元每个字段都有确定的初值
    void hiddenDefaultsAreDefined()
    {
        Network n;
        const int h = n.addNeuron(NeuronKind::Hidden, {});
        const Neuron *p = n.neuron(h);
        QCOMPARE(int(p->activation), int(Activation::Linear));
        QCOMPARE(p->threshold, 0.0);
        QCOMPARE(p->input, 0.0);
        QCOMPARE(p->output, 0.0);
        n.setHiddenParams(h, -0.5, Activation::Linear);   // 没有输入的隐层也照常算 f(0 − θ)
        QVERIFY(n.run().ok);
        QCOMPARE(n.neuron(h)->output, 0.5);
    }

    // 随机分层 DAG（含跳层）：顺序是合法拓扑序，且每个神经元的总输入 = 上游输出 × 权值之和
    void randomDagOrderAndSums()
    {
        Network n;
        std::mt19937 rng(42);
        std::uniform_real_distribution<double> w(-1.0, 1.0);
        std::vector<std::vector<int>> layers(5);
        for (int L = 0; L < 5; ++L)
            for (int k = 0; k < 8; ++k)
                layers[L].push_back(n.addNeuron(L == 0 ? NeuronKind::Input
                                                : L == 4 ? NeuronKind::Output : NeuronKind::Hidden, {}));
        for (int L = 0; L < 4; ++L)
            for (int a : layers[L])
                for (int M = L + 1; M < 5; ++M)
                    for (int b : layers[M])
                        if (rng() % 3 == 0)
                            QVERIFY(n.setWeight(n.addEdge(a, b), w(rng)));
        for (int a : layers[0])
            n.setInputValue(a, w(rng));
        const RunResult r = n.run();
        QVERIFY(r.ok);
        QCOMPARE(r.order.size(), n.neurons().size());
        std::unordered_map<int, int> pos;
        for (int i = 0; i < int(r.order.size()); ++i)
            pos[r.order[i]] = i;
        std::unordered_map<int, double> sum;
        for (const Edge &e : n.edges()) {
            QVERIFY(pos[e.from] < pos[e.to]);
            sum[e.to] += n.neuron(e.from)->output * e.weight;
        }
        for (const Neuron &nu : n.neurons())
            if (nu.kind != NeuronKind::Input)
                QVERIFY(qAbs(nu.input - sum[nu.id]) < 1e-12);
    }

    void reportTextFormat()
    {
        Network n;
        const int i = n.addNeuron(NeuronKind::Input, {});
        const int h = n.addNeuron(NeuronKind::Hidden, {});
        const int o = n.addNeuron(NeuronKind::Output, {});
        n.setWeight(n.addEdge(i, h), 0.5);
        n.setWeight(n.addEdge(h, o), 2);
        n.setInputValue(i, 1);
        n.setHiddenParams(h, 0.1, Activation::Linear);
        n.run();
        const QString t = n.toReportText();
        QVERIFY(t.contains(QStringLiteral("Inputneure:\n神经元编号: 0  输入: 1  输出: 1\n")));
        QVERIFY(t.contains(QStringLiteral("hidden_layer:\n神经元编号: 1  输入: 0.5  输出: 0.4  阈值: 0.1  激活函数编号: 1\n")));
        QVERIFY(t.contains(QStringLiteral("OutputNeure:\n神经元编号: 2  输入: 0.8  输出: 0.8\n")));
        QVERIFY(t.contains(QStringLiteral("wire:\n边的编号: 0  边的权重: 0.5  from 神经元标号: 0  to 神经元标号: 1\n")));
    }

    // 60-60-20 全连接（4800 条边）：run() 应该是毫秒级以下
    void runIsFast()
    {
        Network n;
        std::vector<int> a, b, c;
        for (int k = 0; k < 60; ++k) a.push_back(n.addNeuron(NeuronKind::Input, {}));
        for (int k = 0; k < 60; ++k) b.push_back(n.addNeuron(NeuronKind::Hidden, {}));
        for (int k = 0; k < 20; ++k) c.push_back(n.addNeuron(NeuronKind::Output, {}));
        for (int x : a) for (int y : b) n.setWeight(n.addEdge(x, y), 0.01);
        for (int x : b) for (int y : c) n.setWeight(n.addEdge(x, y), 0.01);
        QCOMPARE(int(n.edges().size()), 4800);
        QElapsedTimer t;
        t.start();
        const int rounds = 100;
        for (int k = 0; k < rounds; ++k)
            n.run();
        const double us = t.nsecsElapsed() / 1000.0 / rounds;
        qInfo("run() on %d neurons / %d edges: %.1f us per run", int(n.neurons().size()),
              int(n.edges().size()), us);
        QVERIFY(us < 20000);
    }
};

// =====================================================================
//  画布交互（放在真正的主窗口里，截图里能看到菜单栏和状态栏）
// =====================================================================
class CanvasTest : public QObject
{
    Q_OBJECT

    MainWindow *w = nullptr;
    Network *net = nullptr;
    NetworkCanvas *canvas = nullptr;

    QString status() const { return w->statusBar()->currentMessage(); }

private slots:
    void init()
    {
        w = new MainWindow;
        w->show();
        QVERIFY(QTest::qWaitForWindowExposed(w));
        QApplication::setActiveWindow(w);   // 窗口级快捷键（Esc）只在活动窗口里生效
        net = &w->network();
        canvas = w->canvas();
    }
    void cleanup()
    {
        delete w;
        w = nullptr;
    }

    // R12：鼠标一次移动很远，神经元照样跟上
    void fastDragFollowsMouse()
    {
        const int id = net->addNeuron(NeuronKind::Input, {100, 100});
        leftDrag(canvas, {100, 100}, {100, 260}, 1);
        QCOMPARE(net->neuron(id)->pos, QPointF(100, 260));
    }

    // R11：叠在一起时只拖走最上面那个
    void overlappingDragMovesOnlyTop()
    {
        const int a = net->addNeuron(NeuronKind::Input, {300, 300});
        const int b = net->addNeuron(NeuronKind::Hidden, {300, 300});   // 后加的画在上面
        leftDrag(canvas, {300, 300}, {300, 460}, 16);
        QCOMPARE(net->neuron(b)->pos, QPointF(300, 460));
        QCOMPARE(net->neuron(a)->pos, QPointF(300, 300));
    }

    // R8：右键从空白处拖到神经元，不生成线
    void rightDragFromEmptySpaceMakesNoEdge()
    {
        const int i = net->addNeuron(NeuronKind::Input, {120, 200});
        const int h = net->addNeuron(NeuronKind::Hidden, {400, 200});
        net->addNeuron(NeuronKind::Output, {650, 420});
        rightDrag(canvas, {120, 200}, {400, 200});
        rightDrag(canvas, {400, 500}, {650, 420});
        canvas->repaint();
        saveShot(w, QStringLiteral("r8_no_phantom_edge"));
        QCOMPARE(edgesOf(*net), QStringLiteral("[%1→%2]").arg(i).arg(h));
    }

    // 右键拖动时有虚线预览，松开在神经元上才连线
    void rightDragShowsPreviewAndCreatesEdge()
    {
        const int i = net->addNeuron(NeuronKind::Input, {150, 300});
        const int o = net->addNeuron(NeuronKind::Output, {600, 300});
        mouse(canvas, QEvent::MouseButtonPress, {150, 300}, Qt::RightButton, Qt::RightButton);
        mouse(canvas, QEvent::MouseMove, {420, 380}, Qt::NoButton, Qt::RightButton);
        QCOMPARE(canvas->linkingFrom(), i);
        canvas->repaint();
        saveShot(w, QStringLiteral("link_preview"));
        mouse(canvas, QEvent::MouseButtonRelease, {600, 300}, Qt::RightButton, Qt::NoButton);
        QCOMPARE(canvas->linkingFrom(), -1);
        QCOMPARE(edgesOf(*net), QStringLiteral("[%1→%2]").arg(i).arg(o));
    }

    // R9：界面上拖出自环会被拒绝，状态栏给出原因
    void selfLoopRejectedInUi()
    {
        const int i = net->addNeuron(NeuronKind::Input, {120, 300});
        const int h = net->addNeuron(NeuronKind::Hidden, {400, 300});
        const int o = net->addNeuron(NeuronKind::Output, {680, 300});
        rightDrag(canvas, {120, 300}, {400, 300});
        rightDrag(canvas, {400, 300}, {680, 300});
        rightDrag(canvas, {400, 300}, {410, 310});   // 自环
        QCOMPARE(status(), Network::errorText(ConnectError::SelfLoop));
        canvas->repaint();
        saveShot(w, QStringLiteral("r9_self_loop_rejected"));
        QCOMPARE(edgesOf(*net), QStringLiteral("[%1→%2, %2→%3]").arg(i).arg(h).arg(o));
    }

    // R1：通过界面删除神经元，删的是它自己和它的线
    void deleteNeuronByClick()
    {
        const int i0 = net->addNeuron(NeuronKind::Input, {100, 150});
        const int i1 = net->addNeuron(NeuronKind::Input, {100, 420});
        const int h = net->addNeuron(NeuronKind::Hidden, {420, 285});
        rightDrag(canvas, {100, 150}, {420, 285});
        rightDrag(canvas, {100, 420}, {420, 285});
        w->findChild<QAction *>(QStringLiteral("actiondeletenode"))->trigger();
        QCOMPARE(canvas->mode(), NetworkCanvas::Mode::DeleteNeuron);
        leftClick(canvas, {100, 150});
        QVERIFY(!net->neuron(i0));
        QCOMPARE(edgesOf(*net), QStringLiteral("[%1→%2]").arg(i1).arg(h));
        QCOMPARE(canvas->mode(), NetworkCanvas::Mode::Normal);
        canvas->repaint();
        saveShot(w, QStringLiteral("r1_delete_neuron"));
    }

    // R10：从右往左的线、竖直的线都能点中
    void deleteRightToLeftAndVerticalEdges()
    {
        net->addNeuron(NeuronKind::Input, {550, 250});
        net->addNeuron(NeuronKind::Hidden, {200, 250});
        rightDrag(canvas, {550, 250}, {200, 250});
        QCOMPARE(int(net->edges().size()), 1);
        const QLineF line = canvas->edgeLine(net->edges().front());
        const QPointF p = line.pointAt(0.25) + QPointF(0, 3);   // 线上 1/4 处偏 3px，避开中间的权值标签
        w->findChild<QAction *>(QStringLiteral("actiondeletewire"))->trigger();
        hover(canvas, p);   // 删除模式下悬停会高亮将被删除的线
        canvas->repaint();
        saveShot(w, QStringLiteral("r10_right_to_left_hover"));
        leftClick(canvas, p);
        QCOMPARE(int(net->edges().size()), 0);

        // 竖直的线：原来斜率公式会除以 0
        net->clear();
        net->addNeuron(NeuronKind::Input, {300, 100});
        net->addNeuron(NeuronKind::Output, {300, 450});
        rightDrag(canvas, {300, 100}, {300, 450});
        QCOMPARE(int(net->edges().size()), 1);
        w->findChild<QAction *>(QStringLiteral("actiondeletewire"))->trigger();
        leftClick(canvas, {303, 200});
        QCOMPARE(int(net->edges().size()), 0);
    }

    // 删除点 / 删除线是两个独立模式；点空白不退出，Esc 和右键退出
    void deleteModesAreSeparateAndCancellable()
    {
        const int i = net->addNeuron(NeuronKind::Input, {150, 300});
        net->addNeuron(NeuronKind::Output, {550, 300});
        rightDrag(canvas, {150, 300}, {550, 300});

        canvas->setMode(NetworkCanvas::Mode::DeleteEdge);
        leftClick(canvas, {150, 300});   // “删除线”模式下点神经元：什么都不删
        QVERIFY(net->neuron(i));
        QCOMPARE(int(net->edges().size()), 1);
        QCOMPARE(canvas->mode(), NetworkCanvas::Mode::DeleteEdge);
        QTest::keyClick(w, Qt::Key_Escape);   // Esc 退出
        QCOMPARE(canvas->mode(), NetworkCanvas::Mode::Normal);

        canvas->setMode(NetworkCanvas::Mode::DeleteNeuron);
        mouse(canvas, QEvent::MouseButtonPress, {700, 500}, Qt::RightButton, Qt::RightButton);   // 右键退出
        mouse(canvas, QEvent::MouseButtonRelease, {700, 500}, Qt::RightButton, Qt::NoButton);
        QCOMPARE(canvas->mode(), NetworkCanvas::Mode::Normal);

        leftClick(canvas, {150, 300});   // 普通模式下点神经元：不删
        QVERIFY(net->neuron(i));

        canvas->setMode(NetworkCanvas::Mode::DeleteNeuron);
        leftClick(canvas, {700, 500});   // 点空白：留在删除模式
        QCOMPARE(canvas->mode(), NetworkCanvas::Mode::DeleteNeuron);
        leftClick(canvas, {150, 300});
        QVERIFY(!net->neuron(i));
        QCOMPARE(int(net->edges().size()), 0);
    }

    void doubleClickEmitsSignals()
    {
        // 测双击信号本身，先断开主窗口里弹对话框的槽
        disconnect(canvas, &NetworkCanvas::neuronActivated, w, nullptr);
        disconnect(canvas, &NetworkCanvas::edgeActivated, w, nullptr);
        const int i = net->addNeuron(NeuronKind::Input, {150, 300});
        net->addNeuron(NeuronKind::Output, {550, 300});
        rightDrag(canvas, {150, 300}, {550, 300});
        QSignalSpy spyN(canvas, &NetworkCanvas::neuronActivated);
        QSignalSpy spyE(canvas, &NetworkCanvas::edgeActivated);
        doubleClick(canvas, {150, 300});
        QCOMPARE(spyN.count(), 1);
        QCOMPARE(spyN.at(0).at(0).toInt(), i);
        doubleClick(canvas, {260, 300});
        QCOMPARE(spyE.count(), 1);
        QCOMPARE(spyE.at(0).at(0).toInt(), net->edges().front().id);
    }

    // R7：空闲时不重绘
    void noRepaintWhenIdle()
    {
        net->addNeuron(NeuronKind::Input, {100, 100});
        net->addNeuron(NeuronKind::Hidden, {300, 100});
        net->addNeuron(NeuronKind::Output, {500, 100});
        canvas->update();
        QTest::qWait(300);
        PaintCounter pc;
        canvas->installEventFilter(&pc);
        const qint64 c0 = cpuMs();
        QTest::qWait(2000);
        const qint64 c1 = cpuMs();
        canvas->removeEventFilter(&pc);
        qInfo("idle 2 s: %d paint events, %lld ms CPU", pc.n, c1 - c0);
        QVERIFY(pc.n <= 2);
    }
};

// =====================================================================
//  主窗口 + 对话框
// =====================================================================
class MainWindowTest : public QObject
{
    Q_OBJECT

private slots:
    // R5：点“取消”什么都不改；对话框打开时预填当前值
    void cancelKeepsValuesAndDialogsArePrefilled()
    {
        MainWindow w;
        w.show();
        Network &n = w.network();
        const int h = n.addNeuron(NeuronKind::Hidden, {300, 300});
        const int i = n.addNeuron(NeuronKind::Input, {100, 300});
        const int o = n.addNeuron(NeuronKind::Output, {550, 300});
        const int e = n.addEdge(h, o);
        n.setHiddenParams(h, 0.5, Activation::Step);
        n.setInputValue(i, 3);
        n.setWeight(e, 2);

        QString shown;
        bool stepChecked = false;
        onNextDialog([&](QDialog *d) {
            shown = d->findChild<QLineEdit *>()->text();
            stepChecked = d->findChild<QRadioButton *>(QStringLiteral("func2"))->isChecked();
            saveShot(d, QStringLiteral("dialog_hidden_prefilled"));
            pressCancel(d);
        });
        w.editNeuron(h);
        QCOMPARE(shown, QStringLiteral("0.5"));
        QVERIFY(stepChecked);
        QCOMPARE(n.neuron(h)->threshold, 0.5);
        QCOMPARE(int(n.neuron(h)->activation), int(Activation::Step));

        onNextDialog([&](QDialog *d) { shown = d->findChild<QLineEdit *>()->text(); pressCancel(d); });
        w.editNeuron(i);
        QCOMPARE(shown, QStringLiteral("3"));
        QCOMPARE(n.neuron(i)->input, 3.0);

        onNextDialog([&](QDialog *d) {
            shown = d->findChild<QLineEdit *>()->text();
            saveShot(d, QStringLiteral("dialog_weight_prefilled"));
            pressCancel(d);
        });
        w.editEdge(e);
        QCOMPARE(shown, QStringLiteral("2"));
        QCOMPARE(n.edge(e)->weight, 2.0);
    }

    void okWritesValues()
    {
        MainWindow w;
        w.show();
        Network &n = w.network();
        const int h = n.addNeuron(NeuronKind::Hidden, {300, 300});
        onNextDialog([](QDialog *d) { fillAndOk(d, QStringLiteral("0.25"), "func2"); });
        w.editNeuron(h);
        QCOMPARE(n.neuron(h)->threshold, 0.25);
        QCOMPARE(int(n.neuron(h)->activation), int(Activation::Step));
    }

    // 填的不是数字：弹提示，对话框不关；改对了再确定
    void invalidNumberKeepsDialogOpen()
    {
        Dialog2 d;
        d.setValue(1);
        bool stillOpen = false;
        QTimer::singleShot(0, [&]() {
            auto *le = d.findChild<QLineEdit *>();
            le->setText(QStringLiteral("abc"));   // setText 绕过校验器，模拟粘贴进来的非法内容
            QTimer::singleShot(0, []() {          // 关掉随后弹出的“输入有误”提示框
                if (auto *mb = qobject_cast<QMessageBox *>(QApplication::activeModalWidget()))
                    mb->accept();
            });
            d.findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click();
            stillOpen = d.isVisible();
            le->setText(QStringLiteral("1.5"));
            d.findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click();
        });
        QCOMPARE(d.exec(), int(QDialog::Accepted));
        QVERIFY(stillOpen);
        QCOMPARE(d.value(), 1.5);
    }

    // R13：对话框用完就释放
    void dialogsAreFreed()
    {
        MainWindow w;
        w.show();
        const int i = w.network().addNeuron(NeuronKind::Input, {200, 200});
        for (int k = 0; k < 5; ++k) {
            onNextDialog([](QDialog *d) { pressCancel(d); });
            w.editNeuron(i);
        }
        QCOMPARE(w.findChildren<QDialog *>().size(), 0);
    }

    void exportWritesUtf8File()
    {
        MainWindow w;
        Network &n = w.network();
        const int i = n.addNeuron(NeuronKind::Input, {});
        const int o = n.addNeuron(NeuronKind::Output, {});
        n.setWeight(n.addEdge(i, o), 0.5);
        n.setInputValue(i, 2);
        n.run();
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("导出.txt"));
        QString err;
        QVERIFY2(w.exportTo(path, &err), qPrintable(err));
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray bytes = f.readAll();
        QVERIFY(bytes.startsWith("\xEF\xBB\xBF"));   // UTF-8 BOM
        const QString text = QString::fromUtf8(bytes.mid(3)).replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
        QCOMPARE(text, n.toReportText());
        QVERIFY(!w.exportTo(dir.filePath(QStringLiteral("不存在的目录/a.txt")), &err));
        QVERIFY(!err.isEmpty());
    }

    // R4 的完整界面流程：新建、拖动、右键连线、双击设参数、运行
    void endToEndSkipConnectionViaUi()
    {
        MainWindow w;
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        NetworkCanvas *c = w.canvas();
        Network &n = w.network();
        auto trigger = [&w](const char *name) { w.findChild<QAction *>(QLatin1String(name))->trigger(); };
        trigger("actionIneure");
        trigger("actionIneure");
        trigger("actionHneure");
        trigger("actionHneure");
        trigger("actionOneure");
        // 新神经元在各自那一列依次往下排，不再摞在同一个点上
        QCOMPARE(n.neuron(0)->pos, QPointF(100, 100));
        QCOMPARE(n.neuron(1)->pos, QPointF(100, 220));
        QCOMPARE(n.neuron(2)->pos, QPointF(300, 100));
        QCOMPARE(n.neuron(3)->pos, QPointF(300, 240));
        QCOMPARE(n.neuron(4)->pos, QPointF(500, 100));

        const QPointF target[5] = {{90, 150}, {90, 450}, {300, 150}, {500, 300}, {700, 300}};
        for (int id : {4, 3, 2, 1, 0})
            leftDrag(c, n.neuron(id)->pos, target[id], 10);
        for (int id = 0; id < 5; ++id)
            QCOMPARE(n.neuron(id)->pos, target[id]);

        rightDrag(c, target[0], target[2]);
        rightDrag(c, target[1], target[3]);
        rightDrag(c, target[2], target[3]);
        rightDrag(c, target[3], target[4]);
        QCOMPARE(edgesOf(n), QStringLiteral("[0→2, 1→3, 2→3, 3→4]"));

        for (int id : {0, 1}) {
            onNextDialog([](QDialog *d) { fillAndOk(d, QStringLiteral("1")); });
            doubleClick(c, target[id]);
        }
        for (int id : {2, 3}) {
            onNextDialog([](QDialog *d) { fillAndOk(d, QStringLiteral("0"), "func1"); });
            doubleClick(c, target[id]);
        }
        std::vector<int> edgeIds;
        for (const Edge &e : n.edges())
            edgeIds.push_back(e.id);
        for (int id : edgeIds) {
            onNextDialog([](QDialog *d) { fillAndOk(d, QStringLiteral("0.4")); });
            doubleClick(c, c->edgeLine(*n.edge(id)).center());
        }
        for (const Edge &e : n.edges())
            QCOMPARE(e.weight, 0.4);

        trigger("actionrunc");
        QCOMPARE(n.neuron(3)->output, 0.56);
        QCOMPARE(n.neuron(4)->output, 0.224);
        QVERIFY(w.statusBar()->currentMessage().contains(QStringLiteral("0 → 1 → 2 → 3 → 4")));
        c->repaint();
        saveShot(&w, QStringLiteral("r4_skip_connection"));
    }
};

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QApplication::setFont(QFont(QStringLiteral("Microsoft YaHei UI"), 9));
    int status = 0;
    {
        NetworkTest t;
        status |= QTest::qExec(&t, argc, argv);
    }
    {
        CanvasTest t;
        status |= QTest::qExec(&t, argc, argv);
    }
    {
        MainWindowTest t;
        status |= QTest::qExec(&t, argc, argv);
    }
    return status;
}

#include "tst_ann.moc"
