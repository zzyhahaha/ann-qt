#ifndef DIALOG2_H
#define DIALOG2_H

#include <QDialog>

namespace Ui {
class Dialog2;
}

// 对话框 2：线的权值
class Dialog2 : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog2(QWidget *parent = nullptr);
    ~Dialog2() override;

    void setValue(double v);
    double value() const;

public slots:
    void accept() override;

private:
    Ui::Dialog2 *ui;
};

#endif // DIALOG2_H
