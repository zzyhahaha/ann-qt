#ifndef NUMBEREDIT_H
#define NUMBEREDIT_H

// 三个对话框共用的“数字输入框”逻辑：输入时就限制只能是数字，按“确定”时再校验一次。
// 原来 toFloat() 不检查是否成功，填错了（或者干脆空着）就悄悄变成 0。

#include <QDoubleValidator>
#include <QLineEdit>
#include <QLocale>
#include <QMessageBox>

namespace NumberEdit {

inline void setup(QLineEdit *edit)
{
    auto *v = new QDoubleValidator(edit);   // 父对象是输入框，随输入框一起销毁
    v->setLocale(QLocale::c());              // 小数点固定是 '.'，和 QString::toDouble 一致
    edit->setValidator(v);
}

inline void setValue(QLineEdit *edit, double value)
{
    edit->setText(QString::number(value, 'g', 10));
    edit->selectAll();                       // 预填当前值并全选，直接输入就是替换
}

inline double value(const QLineEdit *edit)
{
    return edit->text().trimmed().toDouble();
}

// 不是数字：弹提示、选中输入框、返回 false（调用方据此不关闭对话框）
inline bool check(QWidget *parent, QLineEdit *edit, const QString &name)
{
    bool ok = false;
    edit->text().trimmed().toDouble(&ok);
    if (!ok) {
        QMessageBox::warning(parent, QObject::tr("输入有误"),
                             QObject::tr("%1要填一个数，例如 0.5、-1 或 2e-3").arg(name));
        edit->setFocus();
        edit->selectAll();
    }
    return ok;
}

} // namespace NumberEdit

#endif // NUMBEREDIT_H
