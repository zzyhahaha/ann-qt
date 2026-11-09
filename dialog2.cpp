#include "dialog2.h"
#include "ui_dialog2.h"

#include "numberedit.h"

Dialog2::Dialog2(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog2)
{
    ui->setupUi(this);
    NumberEdit::setup(ui->lineEdit);
}

Dialog2::~Dialog2()
{
    delete ui;
}

void Dialog2::setValue(double v)
{
    NumberEdit::setValue(ui->lineEdit, v);
}

double Dialog2::value() const
{
    return NumberEdit::value(ui->lineEdit);
}

void Dialog2::accept()
{
    if (NumberEdit::check(this, ui->lineEdit, tr("权值")))
        QDialog::accept();
}
