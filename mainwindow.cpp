#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <vector>
#include <QPushButton>
#include <QPainter>
#include <QAction>
#include <QToolBar>
using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    resize(800,600);
    vector <InputNeure> INeur;
//    connect(ui->actionIneure,Q,[=](){

//       update();
//    });

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent *){
//指定绘图的设备
    QPainter painter(this);
    QPen pen(QColor(0,0,0));
    pen.setWidth(3);
    painter.setPen(pen);
//    painter.drawEllipse(QPoint(posx,posy),50,50);//画圆


}
