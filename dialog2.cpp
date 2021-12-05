#include "dialog2.h"
#include "ui_dialog2.h"

Dialog2::Dialog2(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog2)
{
    ui->setupUi(this);
}

Dialog2::~Dialog2()
{
    delete ui;
}
//void Dialog2::accept(){
//    QString data =ui->lineEdit->text();
//    emit receiveData(data);
//    QDialog::accept();
//}
QString Dialog2::getinput(){
    QString s=ui->lineEdit->text();
    return s;
}
