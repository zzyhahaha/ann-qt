#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "dialog.h"
#include "dialog2.h"
#include "dialog3.h"
#include "networkcanvas.h"

#include <QAction>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QStatusBar>
#include <QStringConverter>
#include <QTextStream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_canvas(new NetworkCanvas(&m_net, this))
{
    ui->setupUi(this);
    // 原来直接画在 QMainWindow 上，菜单栏会盖住上面一截；现在画布是单独的中心部件
    setCentralWidget(m_canvas);
    resize(800, 600);

    connect(ui->actionIneure, &QAction::triggered, this, [this] { addNeuron(NeuronKind::Input); });
    connect(ui->actionHneure, &QAction::triggered, this, [this] { addNeuron(NeuronKind::Hidden); });
    connect(ui->actionOneure, &QAction::triggered, this, [this] { addNeuron(NeuronKind::Output); });
    connect(ui->actionclr, &QAction::triggered, this, &MainWindow::clearAll);
    connect(ui->actionrunc, &QAction::triggered, this, &MainWindow::runNetwork);
    connect(ui->actionprintdata, &QAction::triggered, this, &MainWindow::exportData);
    // 原来“删除线”“删除点”共用一个 deleteallkind=1，选了删除线照样能删掉点；现在是两个独立的模式
    connect(ui->actiondeletenode, &QAction::triggered, this,
            [this] { m_canvas->setMode(NetworkCanvas::Mode::DeleteNeuron); });
    connect(ui->actiondeletewire, &QAction::triggered, this,
            [this] { m_canvas->setMode(NetworkCanvas::Mode::DeleteEdge); });

    auto *esc = new QAction(this);   // Esc：退出删除模式 / 取消正在拉的线
    esc->setShortcut(Qt::Key_Escape);
    addAction(esc);
    connect(esc, &QAction::triggered, m_canvas, &NetworkCanvas::cancelInteraction);

    connect(m_canvas, &NetworkCanvas::neuronActivated, this, &MainWindow::editNeuron);
    connect(m_canvas, &NetworkCanvas::edgeActivated, this, &MainWindow::editEdge);
    connect(m_canvas, &NetworkCanvas::message, this, &MainWindow::showMessage);

    showMessage(tr("「新建」添加神经元；左键拖动；右键从一个神经元拖到另一个上连线；双击设置参数；最后点「运行」"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showMessage(const QString &text)
{
    statusBar()->showMessage(text);
}

void MainWindow::addNeuron(NeuronKind kind)
{
    const int id = m_net.addNeuron(kind, m_canvas->suggestPosition(kind));
    m_canvas->update();
    showMessage(tr("新建%1神经元 %2").arg(Network::kindName(kind)).arg(id));
}

void MainWindow::clearAll()
{
    if (!m_net.isEmpty()
        && QMessageBox::question(this, tr("清空"), tr("清空全部神经元和连线？这一步不能撤销。"))
               != QMessageBox::Yes)
        return;
    m_net.clear();
    m_canvas->cancelInteraction();
    m_canvas->update();
    showMessage(tr("已清空"));
}

void MainWindow::runNetwork()
{
    const RunResult r = m_net.run();
    m_canvas->update();
    if (!r.ok) {
        QStringList ids;
        for (int id : r.unreached)
            ids << QString::number(id);
        QMessageBox::warning(this, tr("网络有环"),
                             tr("这些神经元在环路上或环路下游，无法计算：%1").arg(ids.join(QStringLiteral(", "))));
        return;
    }
    if (r.order.empty()) {
        showMessage(tr("网络是空的，先「新建」几个神经元"));
        return;
    }
    QStringList ids;
    for (int id : r.order)
        ids << QString::number(id);
    if (ids.size() > 30) {
        ids = ids.mid(0, 30);
        ids << QStringLiteral("…");
    }
    showMessage(tr("运行完成。计算顺序（拓扑序）：%1").arg(ids.join(QStringLiteral(" → "))));
}

void MainWindow::editNeuron(int id)
{
    const Neuron *n = m_net.neuron(id);
    if (!n)
        return;
    // 对话框都建在栈上：函数结束自动销毁（原来每次 new，关掉也不释放）。
    // 只有点“确定”才写回；点“取消”什么都不改（原来取消也会把值清成 0）。
    switch (n->kind) {
    case NeuronKind::Input: {
        Dialog3 dlg(this);
        dlg.setValue(n->input);
        if (dlg.exec() == QDialog::Accepted) {
            m_net.setInputValue(id, dlg.value());
            showMessage(tr("输入神经元 %1 的初值设为 %2，点「运行」重新计算").arg(id).arg(dlg.value()));
        }
        break;
    }
    case NeuronKind::Hidden: {
        Dialog dlg(this);
        dlg.setThreshold(n->threshold);
        dlg.setActivation(n->activation);
        if (dlg.exec() == QDialog::Accepted) {
            m_net.setHiddenParams(id, dlg.threshold(), dlg.activation());
            showMessage(tr("隐层神经元 %1：阈值 %2，激活函数 %3，点「运行」重新计算")
                            .arg(id).arg(dlg.threshold()).arg(int(dlg.activation())));
        }
        break;
    }
    case NeuronKind::Output:
        showMessage(tr("输出神经元的值由「运行」算出来，不用设置"));
        break;
    }
    m_canvas->update();
}

void MainWindow::editEdge(int id)
{
    const Edge *e = m_net.edge(id);
    if (!e)
        return;
    const int from = e->from, to = e->to;
    Dialog2 dlg(this);
    dlg.setValue(e->weight);
    if (dlg.exec() == QDialog::Accepted) {
        m_net.setWeight(id, dlg.value());
        showMessage(tr("线 %1 → %2 的权值设为 %3，点「运行」重新计算").arg(from).arg(to).arg(dlg.value()));
        m_canvas->update();
    }
}

void MainWindow::exportData()
{
    // 原来写死了本机桌面上的 test.txt：换一台电脑就打不开，打不开还接着往里写
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    const QString path = QFileDialog::getSaveFileName(this, tr("导出数据"), dir + QStringLiteral("/ANN数据.txt"),
                                                      tr("文本文件 (*.txt)"));
    if (path.isEmpty())
        return;   // 用户取消
    QString err;
    if (!exportTo(path, &err)) {
        QMessageBox::warning(this, tr("导出失败"), err);
        return;
    }
    showMessage(tr("已导出到 %1").arg(QDir::toNativeSeparators(path)));
}

bool MainWindow::exportTo(const QString &path, QString *error) const
{
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (error)
            *error = tr("无法写入 %1：%2").arg(QDir::toNativeSeparators(path), f.errorString());
        return false;
    }
    QTextStream ts(&f);
    ts.setEncoding(QStringConverter::Utf8);
    ts.setGenerateByteOrderMark(true);   // 带 BOM：老版本记事本打开中文也不乱码
    ts << m_net.toReportText();
    ts.flush();
    if (ts.status() != QTextStream::Ok) {
        if (error)
            *error = tr("写入 %1 时出错").arg(QDir::toNativeSeparators(path));
        return false;
    }
    return true;
}
