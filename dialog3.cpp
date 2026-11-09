#include "dialog3.h"
#include "ui_dialog3.h"

#include "numberedit.h"

Dialog3::Dialog3(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog3)
{
    ui->setupUi(this);
    NumberEdit::setup(ui->lineEdit);
}

Dialog3::~Dialog3()
{
    delete ui;
}

void Dialog3::setValue(double v)
{
    NumberEdit::setValue(ui->lineEdit, v);
}

double Dialog3::value() const
{
    return NumberEdit::value(ui->lineEdit);
}

void Dialog3::accept()
{
    if (NumberEdit::check(this, ui->lineEdit, tr("初值")))
        QDialog::accept();
}
