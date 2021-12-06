#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <cmath>
#include<QMouseEvent>
#include <QMainWindow>
#include <inputneure.h>
#include <hidden_layer.h>
#include <outputneure.h>
#include <transport.h>
#include "inputneure.h"
#include <vector>
#include <adjacency_list.h>
#include <dialog.h>
#include <ui_dialog.h>
#include <dialog2.h>
#include <ui_dialog2.h>
#include <dialog3.h>
#include <ui_dialog3.h>
#include <QDir>

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
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    void drawNeure();
    void drawTran();
    void drawtmp(transport tmp);
    void delInputneure(InputNeure Ineure1);
    void delhiddenneure(hidden_layer Hneure1);
    void delOutputneure(OutputNeure Oneure1);

private:
    Ui::MainWindow *ui;
    Ui::Dialog *id;     //dialog1设置激活函数
    Ui::Dialog2 *id2;   //dialog2设置权值
    Ui::Dialog3 *id3;   //dialog3设置初值


    int  key=0;
    int transkey=0;
    int deleteallkind=0;

    Adjacency_List a;//生成图
    vector<InputNeure> Ineure;
    vector<hidden_layer>Hneure;
    vector<OutputNeure>Oneure;
    vector<transport>tp;
    transport tmp;  //设置线

};
#endif // MAINWINDOW_H
