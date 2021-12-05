#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT
//private slots:
////    void on_Btn_Open_clicked();
////    void on_Btn_Close_clicked();

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();
    QString getinput();
    int getflag();

private:
    Ui::Dialog *ui;

};

#endif // DIALOG_H
