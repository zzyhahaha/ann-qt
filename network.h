#ifndef NETWORK_H
#define NETWORK_H

// 神经网络的数据模型：整个程序里唯一的一份数据。
// 界面（NetworkCanvas）只读这里的数据来画图，所有修改都通过下面的成员函数进行，
// 这样“界面一份、图一份”两边对不上的问题从结构上就不存在了。

#include <QPointF>
#include <QString>
#include <vector>

enum class NeuronKind { Input, Hidden, Output };

// 激活函数，编号沿用原程序（1、2）
enum class Activation {
    Linear = 1,   // 分段线性：把 (总输入 - 阈值) 限制在 [-1, 1]
    Step = 2      // 阶跃：总输入 > 阈值 输出 1，否则 0
};

struct Neuron
{
    int id = -1;
    NeuronKind kind = NeuronKind::Input;
    QPointF pos;                                  // 圆心（画布坐标）
    double input = 0.0;                           // 输入神经元：用户设的初值；其余：上游加权和
    double output = 0.0;
    double threshold = 0.0;                       // 阈值，只对隐层有意义
    Activation activation = Activation::Linear;   // 只对隐层有意义
};

struct Edge
{
    int id = -1;
    int from = -1;                                // 起点神经元 id
    int to = -1;                                  // 终点神经元 id
    double weight = 0.0;
};

// addEdge 失败的原因
enum class ConnectError { None, NoSuchNeuron, FromOutput, ToInput, SelfLoop, Duplicate, Cycle };

struct RunResult
{
    bool ok = true;                 // false = 图里有环（addEdge 已经拦住，这里只是兜底）
    std::vector<int> order;         // 实际计算顺序 = 拓扑序（神经元 id）
    std::vector<int> unreached;     // 有环时没能计算的神经元
};

class Network
{
public:
    // ---- 结构 ----
    int addNeuron(NeuronKind kind, const QPointF &pos);          // 返回新 id
    bool removeNeuron(int id);                                    // 连带删掉所有相连的边
    int addEdge(int from, int to, ConnectError *error = nullptr); // 成功返回边 id，失败返回 -1
    bool removeEdge(int id);
    void clear();                                                 // 清空，id 从 0 重新开始

    // ---- 参数 ----
    bool setPosition(int id, const QPointF &pos);
    bool setInputValue(int id, double value);                     // 只对输入神经元
    bool setHiddenParams(int id, double threshold, Activation f); // 只对隐层神经元
    bool setWeight(int edgeId, double weight);

    // ---- 只读访问 ----
    const Neuron *neuron(int id) const;                           // 找不到返回 nullptr
    const Edge *edge(int id) const;
    const std::vector<Neuron> &neurons() const { return m_neurons; }
    const std::vector<Edge> &edges() const { return m_edges; }
    bool isEmpty() const { return m_neurons.empty(); }

    // ---- 计算 ----
    RunResult run();                                              // Kahn 拓扑排序 + 前向传播
    static double activate(Activation f, double x);
    QString toReportText() const;                                 // “导出数据”的文本，格式同原程序

    static QString errorText(ConnectError e);
    static QString kindName(NeuronKind k);

private:
    Neuron *findNeuron(int id);
    bool reachable(int from, int to) const;                       // 沿边能否从 from 走到 to

    std::vector<Neuron> m_neurons;
    std::vector<Edge> m_edges;
    int m_nextNeuronId = 0;   // 只增不减：删除后编号不复用，界面和计算永远按同一个 id 找东西
    int m_nextEdgeId = 0;
};

#endif // NETWORK_H
