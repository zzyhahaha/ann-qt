#include "dialog.h"
#include "ui_dialog.h"

#include "numberedit.h"

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
    NumberEdit::setup(ui->Yuzhi);
    // 默认选中“线性”。原来两个都不选时 getflag() 返回 0，激活函数直接不起作用
    ui->func1->setChecked(true);
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::setThreshold(double v)
{
    NumberEdit::setValue(ui->Yuzhi, v);
}

double Dialog::threshold() const
{
    return NumberEdit::value(ui->Yuzhi);
}

void Dialog::setActivation(Activation f)
{
    (f == Activation::Step ? ui->func2 : ui->func1)->setChecked(true);
}

Activation Dialog::activation() const
{
    return ui->func2->isChecked() ? Activation::Step : Activation::Linear;
}

void Dialog::accept()
{
    if (NumberEdit::check(this, ui->Yuzhi, tr("阈值")))
        QDialog::accept();
}
