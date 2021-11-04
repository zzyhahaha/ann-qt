#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <inputneure.h>
#include <hidden_layer.h>
#include <outputnerue.h>
#include <transport.h>
#include "inputneure.h"
#include <vector>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void paintEvent(QPaintEvent *);



private:
    Ui::MainWindow *ui;


};
#endif // MAINWINDOW_H
