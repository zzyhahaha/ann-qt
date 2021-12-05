#include "dialog.h"
#include "ui_dialog.h"
#include <ui_mainwindow.h>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
}

Dialog::~Dialog()
{
    delete ui;
}

QString Dialog::getinput(){
    QString s=ui->Yuzhi->text();
    return s;
}
int Dialog::getflag(){
    int Aflag=0;
    if(ui->func1->isChecked()){
        Aflag=1;
        return Aflag;
    }
    else if(ui->func2->isChecked()){
        Aflag=2;
        return Aflag;
    }
    return Aflag;
}
