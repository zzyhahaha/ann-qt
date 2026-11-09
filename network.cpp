#include "network.h"

#include <QTextStream>

#include <algorithm>
#include <queue>
#include <unordered_map>
#include <unordered_set>

static QString num(double v)
{
    return QString::number(v, 'g', 6);
}

// ======================= 结构 =======================

int Network::addNeuron(NeuronKind kind, const QPointF &pos)
{
    Neuron n;
    n.id = m_nextNeuronId++;
    n.kind = kind;
    n.pos = pos;
    m_neurons.push_back(n);
    return n.id;
}

bool Network::removeNeuron(int id)
{
    auto it = std::find_if(m_neurons.begin(), m_neurons.end(),
                           [id](const Neuron &n) { return n.id == id; });
    if (it == m_neurons.end())
        return false;
    m_neurons.erase(it);

    // erase-remove 惯用法：remove_if 把要留下的边挪到前面并返回新结尾，erase 一次截掉尾巴。
    // 只遍历一遍，不会像原来“erase 后 i=0、再被 i++”那样跳过元素，也不会越界。
    m_edges.erase(std::remove_if(m_edges.begin(), m_edges.end(),
                                 [id](const Edge &e) { return e.from == id || e.to == id; }),
                  m_edges.end());
    return true;
}

int Network::addEdge(int from, int to, ConnectError *error)
{
    auto fail = [error](ConnectError e) {
        if (error)
            *error = e;
        return -1;
    };
    const Neuron *a = neuron(from);
    const Neuron *b = neuron(to);
    if (!a || !b)
        return fail(ConnectError::NoSuchNeuron);
    if (from == to)
        return fail(ConnectError::SelfLoop);
    if (a->kind == NeuronKind::Output)
        return fail(ConnectError::FromOutput);
    if (b->kind == NeuronKind::Input)
        return fail(ConnectError::ToInput);
    for (const Edge &e : m_edges)
        if (e.from == from && e.to == to)
            return fail(ConnectError::Duplicate);
    // 加上 from→to 后成环 ⇔ 原图里已经能从 to 走回 from
    if (reachable(to, from))
        return fail(ConnectError::Cycle);

    Edge e;
    e.id = m_nextEdgeId++;
    e.from = from;
    e.to = to;
    m_edges.push_back(e);
    if (error)
        *error = ConnectError::None;
    return e.id;
}

bool Network::removeEdge(int id)
{
    auto it = std::find_if(m_edges.begin(), m_edges.end(), [id](const Edge &e) { return e.id == id; });
    if (it == m_edges.end())
        return false;
    m_edges.erase(it);
    return true;
}

void Network::clear()
{
    m_neurons.clear();
    m_edges.clear();
    m_nextNeuronId = 0;   // 两个计数器一起归零（原来 transkey 漏了，清空后边的编号就错位）
    m_nextEdgeId = 0;
}

// ======================= 参数 =======================

bool Network::setPosition(int id, const QPointF &pos)
{
    Neuron *n = findNeuron(id);
    if (!n)
        return false;
    n->pos = pos;
    return true;
}

bool Network::setInputValue(int id, double value)
{
    Neuron *n = findNeuron(id);
    if (!n || n->kind != NeuronKind::Input)
        return false;
    n->input = value;
    n->output = value;   // 输入神经元的输出就是输入，设完马上能看到
    return true;
}

bool Network::setHiddenParams(int id, double threshold, Activation f)
{
    Neuron *n = findNeuron(id);
    if (!n || n->kind != NeuronKind::Hidden)
        return false;
    n->threshold = threshold;
    n->activation = f;
    return true;
}

bool Network::setWeight(int edgeId, double weight)
{
    auto it = std::find_if(m_edges.begin(), m_edges.end(),
                           [edgeId](const Edge &e) { return e.id == edgeId; });
    if (it == m_edges.end())
        return false;
    it->weight = weight;
    return true;
}

// ======================= 查找 =======================

const Neuron *Network::neuron(int id) const
{
    auto it = std::find_if(m_neurons.begin(), m_neurons.end(),
                           [id](const Neuron &n) { return n.id == id; });
    return it == m_neurons.end() ? nullptr : &*it;
}

Neuron *Network::findNeuron(int id)
{
    return const_cast<Neuron *>(static_cast<const Network *>(this)->neuron(id));
}

const Edge *Network::edge(int id) const
{
    auto it = std::find_if(m_edges.begin(), m_edges.end(), [id](const Edge &e) { return e.id == id; });
    return it == m_edges.end() ? nullptr : &*it;
}

bool Network::reachable(int from, int to) const
{
    if (from == to)
        return true;
    // 起点没有出边就哪也去不了：新神经元连线时最常见，省掉下面建表
    if (std::none_of(m_edges.begin(), m_edges.end(), [from](const Edge &e) { return e.from == from; }))
        return false;
    // 广度优先搜索（BFS），O(V+E)
    std::unordered_map<int, std::vector<int>> next;
    for (const Edge &e : m_edges)
        next[e.from].push_back(e.to);
    std::queue<int> q;
    std::unordered_set<int> seen{from};
    q.push(from);
    while (!q.empty()) {
        const int u = q.front();
        q.pop();
        if (u == to)
            return true;
        for (int v : next[u])
            if (seen.insert(v).second)
                q.push(v);
    }
    return false;
}

// ======================= 计算 =======================

double Network::activate(Activation f, double x)
{
    switch (f) {
    case Activation::Linear:
        return std::clamp(x, -1.0, 1.0);
    case Activation::Step:
        return x > 0.0 ? 1.0 : 0.0;
    }
    return x;
}

RunResult Network::run()
{
    RunResult result;
    const int n = int(m_neurons.size());

    // 1. id → 下标，建一次，后面查找都是 O(1)
    std::unordered_map<int, int> index;
    index.reserve(n);
    for (int i = 0; i < n; ++i)
        index[m_neurons[i].id] = i;

    // 2. 从边表临时建出邻接表和入度表：O(V+E)。用完即弃，不长期保存第二份数据，也就谈不上不同步
    std::vector<std::vector<int>> out(n);   // out[i] = 从第 i 个神经元出发的边（m_edges 的下标）
    std::vector<int> indegree(n, 0);
    for (int k = 0; k < int(m_edges.size()); ++k) {
        out[index.at(m_edges[k].from)].push_back(k);
        ++indegree[index.at(m_edges[k].to)];
    }

    // 3. 清掉上一次的结果，避免断开的神经元还显示旧值
    for (Neuron &nu : m_neurons) {
        if (nu.kind == NeuronKind::Input)
            nu.output = nu.input;
        else
            nu.input = nu.output = 0.0;
    }

    // 4. Kahn 算法：入度为 0 的先算。每个神经元只在入度减到 0 的那一刻入队一次，
    //    出队时它的上游一定都算完了，input 就是完整的加权和
    std::queue<int> ready;
    for (int i = 0; i < n; ++i)
        if (indegree[i] == 0)
            ready.push(i);
    while (!ready.empty()) {
        const int u = ready.front();
        ready.pop();
        Neuron &cur = m_neurons[u];
        if (cur.kind == NeuronKind::Hidden)
            cur.output = activate(cur.activation, cur.input - cur.threshold);   // M-P 模型：f(Σwx − θ)
        else if (cur.kind == NeuronKind::Output)
            cur.output = cur.input;
        result.order.push_back(cur.id);
        for (int k : out[u]) {
            const int v = index.at(m_edges[k].to);
            m_neurons[v].input += cur.output * m_edges[k].weight;
            if (--indegree[v] == 0)
                ready.push(v);
        }
    }

    // 5. 算到的个数少于总数 ⇒ 有环（原程序数了 cnt 却从没检查）
    if (int(result.order.size()) < n) {
        result.ok = false;
        for (int i = 0; i < n; ++i) {
            if (indegree[i] > 0) {
                result.unreached.push_back(m_neurons[i].id);
                m_neurons[i].input = m_neurons[i].output = 0.0;
            }
        }
    }
    return result;
}

QString Network::toReportText() const
{
    // 格式与原程序“输出txt”保持一致，方便和以前导出的文件对照
    QString s;
    QTextStream ts(&s);
    ts << '\n' << "Inputneure:" << '\n';
    for (const Neuron &n : m_neurons)
        if (n.kind == NeuronKind::Input)
            ts << "神经元编号: " << n.id << "  输入: " << num(n.input) << "  输出: " << num(n.output) << '\n';
    ts << '\n' << "hidden_layer:" << '\n';
    for (const Neuron &n : m_neurons)
        if (n.kind == NeuronKind::Hidden)
            ts << "神经元编号: " << n.id << "  输入: " << num(n.input) << "  输出: " << num(n.output)
               << "  阈值: " << num(n.threshold) << "  激活函数编号: " << int(n.activation) << '\n';
    ts << '\n' << "OutputNeure:" << '\n';
    for (const Neuron &n : m_neurons)
        if (n.kind == NeuronKind::Output)
            ts << "神经元编号: " << n.id << "  输入: " << num(n.input) << "  输出: " << num(n.output) << '\n';
    ts << '\n' << "wire:" << '\n';
    for (const Edge &e : m_edges)
        ts << "边的编号: " << e.id << "  边的权重: " << num(e.weight) << "  from 神经元标号: " << e.from
           << "  to 神经元标号: " << e.to << '\n';
    ts.flush();
    return s;
}

QString Network::errorText(ConnectError e)
{
    switch (e) {
    case ConnectError::None:         return QString();
    case ConnectError::NoSuchNeuron: return QStringLiteral("神经元不存在");
    case ConnectError::SelfLoop:     return QStringLiteral("不能连到自己");
    case ConnectError::FromOutput:   return QStringLiteral("输出神经元不能作为连线起点");
    case ConnectError::ToInput:      return QStringLiteral("不能连到输入神经元");
    case ConnectError::Duplicate:    return QStringLiteral("这两个神经元之间已经有这条线了");
    case ConnectError::Cycle:        return QStringLiteral("这条线会形成环路：前馈网络不能有环，否则无法按拓扑序计算");
    }
    return QString();
}

QString Network::kindName(NeuronKind k)
{
    switch (k) {
    case NeuronKind::Input:  return QStringLiteral("输入");
    case NeuronKind::Hidden: return QStringLiteral("隐层");
    case NeuronKind::Output: return QStringLiteral("输出");
    }
    return QString();
}
