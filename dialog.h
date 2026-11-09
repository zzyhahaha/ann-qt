#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

#include "network.h"

namespace Ui {
class Dialog;
}

// 对话框 1：隐层神经元的阈值和激活函数
class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog() override;

    void setThreshold(double v);
    double threshold() const;
    void setActivation(Activation f);
    Activation activation() const;

public slots:
    void accept() override;   // 阈值不是数字时提示，并留在对话框里

private:
    Ui::Dialog *ui;
};

#endif // DIALOG_H
