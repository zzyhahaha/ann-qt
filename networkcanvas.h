#ifndef NETWORKCANVAS_H
#define NETWORKCANVAS_H

// 画布：把 Network 画出来，并处理鼠标操作（拖动、右键连线、删除、双击）。
// 它不存任何神经元数据，只存“交互状态”：正在拖谁、正在从谁连线、当前处于哪种模式。

#include <QLineF>
#include <QRectF>
#include <QWidget>

#include <unordered_map>

#include "network.h"

class NetworkCanvas : public QWidget
{
    Q_OBJECT

public:
    enum class Mode { Normal, DeleteNeuron, DeleteEdge };
    Q_ENUM(Mode)

    explicit NetworkCanvas(Network *net, QWidget *parent = nullptr);

    Mode mode() const { return m_mode; }
    void setMode(Mode m);
    void cancelInteraction();                         // Esc：退出删除模式 / 取消正在拉的线

    int neuronAt(const QPointF &p) const;             // 鼠标下最上面的神经元，没有返回 -1
    int edgeAt(const QPointF &p) const;               // 离 p 最近且在容差内的线，没有返回 -1
    QPointF suggestPosition(NeuronKind kind) const;   // 新神经元放哪：本列往下找空位
    QLineF edgeLine(const Edge &e) const;             // 画出来的那段线（两个圆周之间）

    static double radius(NeuronKind kind);            // 输入/输出 50，隐层 60（沿用原程序）

    int draggingId() const { return m_dragId; }
    int linkingFrom() const { return m_linkFrom; }

signals:
    void neuronActivated(int id);                     // 双击了神经元
    void edgeActivated(int id);                       // 双击了线
    void networkEdited();                             // 加线 / 删点 / 删线之后
    void message(const QString &text);                // 给状态栏的提示
    void modeChanged(NetworkCanvas::Mode mode);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    std::unordered_map<int, const Neuron *> byId() const;
    void drawNeuron(QPainter &p, const Neuron &n, bool highlighted) const;
    QRectF weightLabelRect(const Edge &e, const QLineF &line) const;
    QPointF clampToCanvas(NeuronKind kind, const QPointF &p) const;
    void updateHover(const QPointF &p);

    Network *m_net;
    Mode m_mode = Mode::Normal;
    int m_dragId = -1;          // 正在拖动的神经元：按下时记下 id，拖动过程中不再重新判断命中
    QPointF m_dragOffset;       // 按下点相对圆心的偏移，拖动时圆心不会跳到鼠标上
    int m_linkFrom = -1;        // 正在连线的起点；只有在神经元上按下右键才会设置
    QPointF m_linkPos;          // 连线时鼠标的位置（画虚线预览）
    int m_hoverNeuron = -1;     // 删除模式下鼠标指着的对象，高亮提示
    int m_hoverEdge = -1;
};

#endif // NETWORKCANVAS_H
