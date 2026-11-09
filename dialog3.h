#ifndef DIALOG3_H
#define DIALOG3_H

#include <QDialog>

namespace Ui {
class Dialog3;
}

// 对话框 3：输入神经元的初值
class Dialog3 : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog3(QWidget *parent = nullptr);
    ~Dialog3() override;

    void setValue(double v);
    double value() const;

public slots:
    void accept() override;

private:
    Ui::Dialog3 *ui;
};

#endif // DIALOG3_H
