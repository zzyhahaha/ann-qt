#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "network.h"

class NetworkCanvas;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// 主窗口：菜单、对话框、导出。画图和鼠标交互交给 NetworkCanvas，数据只有 m_net 一份。
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    Network &network() { return m_net; }
    NetworkCanvas *canvas() const { return m_canvas; }

    void addNeuron(NeuronKind kind);
    void runNetwork();
    void editNeuron(int id);
    void editEdge(int id);
    bool exportTo(const QString &path, QString *error = nullptr) const;

private:
    void clearAll();
    void exportData();
    void showMessage(const QString &text);

    Ui::MainWindow *ui;
    Network m_net;              // 唯一的数据
    NetworkCanvas *m_canvas;    // 只负责显示和交互
};

#endif // MAINWINDOW_H
