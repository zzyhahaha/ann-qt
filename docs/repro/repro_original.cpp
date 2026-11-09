// repro_original.cpp
// 在原版代码（git HEAD 4ffc9e2，2021-12-07）上逐条复现 bug，不改原版任何一行。
// 做法：用 QMouseEvent 模拟鼠标、用定时器自动填写/关闭对话框，跑完检查内部数据并截图。
// 用法：set QT_QPA_PLATFORM=offscreen && repro_original.exe <输出目录>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <QtWidgets>
#include <functional>
#include <vector>
#include <queue>
#include <string>
#include <iostream>
#include <cmath>
#include <cassert>

#include "network.h"   // 新版模型，只用于下面的基准对比（放在 #define private public 之前）

// 只是为了读私有成员（tp、Ineure、a.tr……）做检查，测试里常用的手段，不改变原版逻辑
#define private public
#define protected public
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "adjacency_list.h"
#undef private
#undef protected

static QString g_out;
static QStringList g_report;
static int g_reproduced = 0, g_total = 0;

static void report(const QString &id, const QString &title, bool reproduced,
                   const QString &expected, const QString &actual)
{
    ++g_total;
    if (reproduced) ++g_reproduced;
    g_report << QString("[%1] %2 %3").arg(reproduced ? "复现" : "未复现", id, title)
             << "    期望: " + expected
             << "    实际: " + actual << "";
}

// ---------- 鼠标 ----------
static void mouse(QWidget *w, QEvent::Type t, QPoint p, Qt::MouseButton b, Qt::MouseButtons bs)
{
    QMouseEvent ev(t, QPointF(p), QPointF(w->mapToGlobal(p)), b, bs, Qt::NoModifier);
    QApplication::sendEvent(w, &ev);
}
static void rightDrag(QWidget *w, QPoint a, QPoint b)
{
    mouse(w, QEvent::MouseButtonPress, a, Qt::RightButton, Qt::RightButton);
    mouse(w, QEvent::MouseMove, b, Qt::NoButton, Qt::RightButton);
    mouse(w, QEvent::MouseButtonRelease, b, Qt::RightButton, Qt::NoButton);
}
static void leftClick(QWidget *w, QPoint p)
{
    mouse(w, QEvent::MouseButtonPress, p, Qt::LeftButton, Qt::LeftButton);
    mouse(w, QEvent::MouseButtonRelease, p, Qt::LeftButton, Qt::NoButton);
}
static void leftDrag(QWidget *w, QPoint a, QPoint b, int steps)
{
    mouse(w, QEvent::MouseButtonPress, a, Qt::LeftButton, Qt::LeftButton);
    for (int i = 1; i <= steps; ++i) {
        QPoint p = a + (b - a) * i / steps;
        mouse(w, QEvent::MouseMove, p, Qt::NoButton, Qt::LeftButton);
    }
    mouse(w, QEvent::MouseButtonRelease, b, Qt::LeftButton, Qt::NoButton);
}
static void doubleClick(QWidget *w, QPoint p)
{
    leftClick(w, p);
    mouse(w, QEvent::MouseButtonDblClick, p, Qt::LeftButton, Qt::LeftButton);
    mouse(w, QEvent::MouseButtonRelease, p, Qt::LeftButton, Qt::NoButton);
}

// ---------- 对话框：预约“下一个弹出的模态对话框”怎么处理 ----------
static void onNextDialog(std::function<void(QDialog *)> fn)
{
    QTimer::singleShot(0, [fn]() {
        QDialog *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (!d) {
            const auto tops = QApplication::topLevelWidgets();
            for (QWidget *t : tops) {
                auto *dd = qobject_cast<QDialog *>(t);
                if (dd && dd->isVisible()) { d = dd; break; }
            }
        }
        if (d) fn(d);
    });
}
static void fillAndOk(QDialog *d, const QString &text, const char *radio = nullptr)
{
    if (auto *le = d->findChild<QLineEdit *>()) le->setText(text);
    if (radio)
        if (auto *rb = d->findChild<QRadioButton *>(radio)) rb->setChecked(true);
    d->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click();
}
static void pressCancel(QDialog *d)
{
    d->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Cancel)->click();
}

// ---------- 杂项 ----------
static void wait(int ms)
{
    QEventLoop loop;
    QTimer::singleShot(ms, &loop, &QEventLoop::quit);
    loop.exec();
}
static qint64 cpuMs()
{
    FILETIME c, e, k, u;
    GetProcessTimes(GetCurrentProcess(), &c, &e, &k, &u);
    auto ms = [](FILETIME f) {
        ULARGE_INTEGER x; x.LowPart = f.dwLowDateTime; x.HighPart = f.dwHighDateTime;
        return qint64(x.QuadPart / 10000);
    };
    return ms(k) + ms(u);
}
class PaintCounter : public QObject {
public:
    int n = 0;
    bool eventFilter(QObject *, QEvent *e) override { if (e->type() == QEvent::Paint) ++n; return false; }
};
static MainWindow *newWindow()
{
    auto *w = new MainWindow;
    w->show();
    QCoreApplication::processEvents();
    return w;
}
static void shot(QWidget *w, const QString &name)
{
    w->grab().save(g_out + "/shots/before_" + name + ".png");
}
static QString edgesGui(MainWindow *w)
{
    QStringList s;
    for (auto &t : w->tp) s << QString("%1→%2").arg(t.getfrom()).arg(t.getto());
    return "[" + s.join(", ") + "]";
}
static QString edgesGraph(MainWindow *w)
{
    QStringList s;
    for (auto &t : w->a.tr) s << QString("%1→%2").arg(t.getfrom()).arg(t.getto());
    return "[" + s.join(", ") + "]";
}
static QPoint mid(const transport &t)
{
    transport c = t;
    return QPoint((c.getposx1() + c.getposx2()) / 2, (c.getposy1() + c.getposy2()) / 2);
}

// ======================= 各条 bug =======================

// R1 删除神经元：先 erase 再用 Ineure[i]，删的是“下一个”神经元的线
static void r1()
{
    MainWindow *w = newWindow();
    w->ui->actionIneure->trigger();   // key 0
    w->ui->actionIneure->trigger();   // key 1
    w->ui->actionHneure->trigger();   // key 2
    w->Ineure[0].setpos(100, 150);
    w->Ineure[1].setpos(100, 420);
    w->Hneure[0].setpos(420, 285);
    rightDrag(w, {100, 150}, {420, 285});
    rightDrag(w, {100, 420}, {420, 285});
    shot(w, "r1_1_before_delete");
    w->ui->actiondeletenode->trigger();
    leftClick(w, {100, 150});         // 删 0 号输入神经元
    shot(w, "r1_2_after_delete");
    QString gui = edgesGui(w), graph = edgesGraph(w);
    report("R1", "删除神经元时删错了线（erase 之后又用 Ineure[i]）", gui == "[0→2]",
           "界面和图里都只剩 [1→2]",
           "界面剩 " + gui + "：0 号已经删了，它的线还挂在原处；1 号的线反而被删。图里剩 " + graph
               + "，两份数据从此不一致");
    delete w;
}

// R2 删除循环：erase 后 i=0，接着 for 的 i++ 把 i 变成 1，跳过了新的 tp[0]
static void r2()
{
    MainWindow *w = newWindow();
    w->ui->actionHneure->trigger();   // key 0
    w->ui->actionOneure->trigger();   // key 1
    w->ui->actionOneure->trigger();   // key 2
    w->Hneure[0].setpos(250, 300);
    w->Oneure[0].setpos(600, 180);
    w->Oneure[1].setpos(600, 420);
    rightDrag(w, {250, 300}, {600, 180});
    rightDrag(w, {250, 300}, {600, 420});
    hidden_layer h = w->Hneure[0];
    w->delhiddenneure(h);             // 用正确的神经元直接调用，单看循环本身
    QString gui = edgesGui(w);
    report("R2", "删除循环 erase 后写 i=0，再被 i++ 跳过一个元素", gui != "[]",
           "[]（隐层神经元的两条出线都删掉）", gui + "（漏删一条）");
    delete w;
}

// R3 删过线以后：界面边编号用 transkey，图里用 m（边数），两套编号对不上
static void r3()
{
    MainWindow *w = newWindow();
    w->ui->actionIneure->trigger();   // 0
    w->ui->actionOneure->trigger();   // 1
    w->Ineure[0].setpos(150, 300);
    w->Oneure[0].setpos(550, 300);
    rightDrag(w, {150, 300}, {550, 300});      // 界面 key0 / 图 key0
    w->ui->actiondeletewire->trigger();
    leftClick(w, {350, 300});                  // 删掉这条线
    rightDrag(w, {150, 300}, {550, 300});      // 重连：界面 key1 / 图 key0
    onNextDialog([](QDialog *d) { fillAndOk(d, "1"); });
    doubleClick(w, {150, 300});                // 输入 = 1
    onNextDialog([](QDialog *d) { fillAndOk(d, "2"); });
    doubleClick(w, {350, 300});                // 权值 = 2
    w->ui->actionrunc->trigger();
    shot(w, "r3_weight_lost_after_delete");
    float out = w->Oneure[0].output();
    report("R3", "删过一条线后，新连的线设的权值不参与计算", std::fabs(out - 2.0f) > 1e-6f,
           "输出 = 1 × 2 = 2",
           QString("输出 = %1。界面那条边 key=%2、权值 %3；图里那条边 key=%4、权值 %5（按 key 找不到，权值没传过去）")
               .arg(out).arg(w->tp[0].getkey()).arg(w->tp[0].getWeight())
               .arg(w->a.tr[0].getkey()).arg(w->a.tr[0].getWeight()));
    delete w;
}

// R3b 清空后 transkey 没归零，同样的编号错位
static void r3b()
{
    MainWindow *w = newWindow();
    auto build = [w]() {
        w->ui->actionIneure->trigger();
        w->ui->actionOneure->trigger();
        w->Ineure[0].setpos(150, 300);
        w->Oneure[0].setpos(550, 300);
        rightDrag(w, {150, 300}, {550, 300});
        onNextDialog([](QDialog *d) { fillAndOk(d, "1"); });
        doubleClick(w, {150, 300});
        onNextDialog([](QDialog *d) { fillAndOk(d, "2"); });
        doubleClick(w, {350, 300});
        w->ui->actionrunc->trigger();
        return w->Oneure[0].output();
    };
    float first = build();
    w->ui->actionclr->trigger();
    float second = build();
    report("R3b", "「清空」后重新搭同一个网络，结果变 0（transkey 没归零）",
           std::fabs(first - 2) < 1e-6f && std::fabs(second - 2) > 1e-6f, "两次都是 2",
           QString("第一次 %1，清空后第二次 %2").arg(first).arg(second));
    delete w;
}

// R4 拓扑排序：节点重复入队 + 入度被重复减，有“跳层连接”时用到旧的中间值
static void r4()
{
    Adjacency_List g;
    for (int k = 0; k < 5; ++k) g.addvertax(k);
    g.addInput(InputNeure(0));
    g.addInput(InputNeure(1));
    hidden_layer h2(2), h3(3);
    h2.setjiflag(1); h3.setjiflag(1); h2.setthr(0); h3.setthr(0);
    g.addhidden(h2);
    g.addhidden(h3);
    g.addOutput(OutputNeure(4));
    int ed[4][2] = {{0, 2}, {1, 3}, {2, 3}, {3, 4}};
    for (auto &e : ed) { transport t; t.setfrom(e[0]); t.setto(e[1]); g.addEdge(t); }
    std::vector<transport> tv(4);
    for (int i = 0; i < 4; ++i) { tv[i].setkey(i); tv[i].setWeight(0.4f); g.intransport(tv.begin() + i); }
    std::vector<InputNeure> iv{InputNeure(0), InputNeure(1)};
    for (int i = 0; i < 2; ++i) { iv[i].setInput(1); g.inInput(iv.begin() + i); }
    g.TopSort();
    float h3out = g.hidden[1].Output(), o4 = g.Outne[0].output();
    report("R4", "拓扑排序在有跳层连接时算错（0→2, 1→3, 2→3, 3→4，权值都是 0.4，输入都是 1）",
           std::fabs(o4 - 0.224f) > 1e-5f,
           "H2 = 0.4，H3 = 0.4 + 0.4×0.4 = 0.56，O4 = 0.56×0.4 = 0.224",
           QString("H3 显示 %1（对），但 O4 = %2 = 0.4×0.4，用的是 H3 只收到一半输入时的旧值")
               .arg(h3out).arg(o4));
}

// R4 的界面版，留一张截图
static void r4gui()
{
    MainWindow *w = newWindow();
    w->ui->actionIneure->trigger();   // 0
    w->ui->actionIneure->trigger();   // 1
    w->ui->actionHneure->trigger();   // 2
    w->ui->actionHneure->trigger();   // 3
    w->ui->actionOneure->trigger();   // 4
    w->Ineure[0].setpos(90, 150);
    w->Ineure[1].setpos(90, 450);
    w->Hneure[0].setpos(300, 150);
    w->Hneure[1].setpos(500, 300);
    w->Oneure[0].setpos(700, 300);
    rightDrag(w, {90, 150}, {300, 150});
    rightDrag(w, {90, 450}, {500, 300});
    rightDrag(w, {300, 150}, {500, 300});
    rightDrag(w, {500, 300}, {700, 300});
    onNextDialog([](QDialog *d) { fillAndOk(d, "1"); });
    doubleClick(w, {90, 150});
    onNextDialog([](QDialog *d) { fillAndOk(d, "1"); });
    doubleClick(w, {90, 450});
    onNextDialog([](QDialog *d) { fillAndOk(d, "0", "func1"); });
    doubleClick(w, {300, 150});
    onNextDialog([](QDialog *d) { fillAndOk(d, "0", "func1"); });
    doubleClick(w, {500, 300});
    for (size_t i = 0; i < w->tp.size(); ++i) {
        QPoint m = mid(w->tp[i]);
        onNextDialog([](QDialog *d) { fillAndOk(d, "0.4"); });
        doubleClick(w, m);
    }
    w->ui->actionrunc->trigger();
    shot(w, "r4_skip_connection");
    report("R4-界面", "同一个网络在界面里搭一遍再运行", std::fabs(w->Oneure[0].output() - 0.224f) > 1e-5f,
           "O4 = 0.224",
           QString("H3 = %1，O4 = %2").arg(w->Hneure[1].Output()).arg(w->Oneure[0].output()));
    delete w;
}

// R5 对话框按“取消”也会把值写回去（空串 → 0，单选框都没选 → 0）
static void r5()
{
    MainWindow *w = newWindow();
    w->ui->actionHneure->trigger();
    w->Hneure[0].setpos(300, 300);
    onNextDialog([](QDialog *d) { fillAndOk(d, "0.5", "func2"); });
    doubleClick(w, {300, 300});
    float t1 = w->Hneure[0].getthr();
    int f1 = w->Hneure[0].getjiflag();
    onNextDialog([](QDialog *d) { pressCancel(d); });
    doubleClick(w, {300, 300});
    float t2 = w->Hneure[0].getthr();
    int f2 = w->Hneure[0].getjiflag();
    w->ui->actionIneure->trigger();
    w->Ineure[0].setpos(100, 450);
    onNextDialog([](QDialog *d) { fillAndOk(d, "3"); });
    doubleClick(w, {100, 450});
    float i1 = w->Ineure[0].getInput();
    onNextDialog([](QDialog *d) { pressCancel(d); });
    doubleClick(w, {100, 450});
    float i2 = w->Ineure[0].getInput();
    report("R5", "对话框点「取消」，原来的值被清成 0", t2 == 0 && f2 == 0 && i2 == 0,
           "取消 = 什么都不改：阈值 0.5、激活函数 2、输入 3",
           QString("确定后：阈值 %1、激活函数 %2、输入 %3；取消后：阈值 %4、激活函数 %5、输入 %6")
               .arg(t1).arg(f1).arg(i1).arg(t2).arg(f2).arg(i2));
    delete w;
}

// R6 新建隐层神经元的激活函数编号没初始化
static void r6()
{
    MainWindow *w = newWindow();
    w->ui->actionHneure->trigger();
    int g = w->Hneure[0].getjiflag(), gg = w->a.hidden[0].getjiflag();
    report("R6", "新建隐层神经元的激活函数编号（jiflag）没初始化", !(g == 1 || g == 2) || !(gg == 1 || gg == 2),
           "新建时就有确定的默认值（1 或 2）",
           QString("界面那份读到 %1，图里那份读到 %2（读未初始化变量是未定义行为，每次可能不同；"
                   "不是 1/2 时激活函数直接不起作用）").arg(g).arg(gg));
    delete w;
}

// R7 paintEvent 里调用 update()：画完马上又排下一次重绘，空闲也停不下来
static void r7()
{
    MainWindow *w = newWindow();
    w->ui->actionIneure->trigger();
    w->ui->actionHneure->trigger();
    w->ui->actionOneure->trigger();
    wait(300);
    PaintCounter pc;
    w->installEventFilter(&pc);
    qint64 c0 = cpuMs();
    wait(2000);
    qint64 c1 = cpuMs();
    w->removeEventFilter(&pc);
    report("R7", "paintEvent 里调用 update()，什么都不做也一直重绘", pc.n > 50,
           "空闲 2 秒内重绘 0～几次，CPU 接近 0",
           QString("空闲 2 秒内重绘 %1 次，进程用掉 CPU 时间 %2 ms").arg(pc.n).arg(c1 - c0));
    delete w;
}

// R8 右键从空白处拖到神经元，也会连出一条线，起点沿用上一次的
static void r8()
{
    MainWindow *w = newWindow();
    w->ui->actionIneure->trigger();   // 0
    w->ui->actionHneure->trigger();   // 1
    w->ui->actionOneure->trigger();   // 2
    w->Ineure[0].setpos(120, 200);
    w->Hneure[0].setpos(400, 200);
    w->Oneure[0].setpos(650, 420);
    rightDrag(w, {120, 200}, {400, 200});   // 正常连线 0→1
    rightDrag(w, {400, 500}, {650, 420});   // 从空白处拖到输出神经元
    shot(w, "r8_phantom_edge");
    QString gui = edgesGui(w);
    report("R8", "右键从空白处拖到神经元，也会生成一条线", w->tp.size() == 2,
           "只有 [0→1]，第二次拖动不生成线", gui + "（多出的 0→2 起点是上一次连线留下的）");
    delete w;
}

// R9 可以连自环，运行时该神经元入度永远减不到 0，下游全是 0，也没有任何提示
static void r9()
{
    MainWindow *w = newWindow();
    w->ui->actionIneure->trigger();   // 0
    w->ui->actionHneure->trigger();   // 1
    w->ui->actionOneure->trigger();   // 2
    w->Ineure[0].setpos(120, 300);
    w->Hneure[0].setpos(400, 300);
    w->Oneure[0].setpos(680, 300);
    rightDrag(w, {120, 300}, {400, 300});
    rightDrag(w, {400, 300}, {680, 300});
    rightDrag(w, {400, 300}, {410, 310});   // 自环 1→1
    onNextDialog([](QDialog *d) { fillAndOk(d, "1"); });
    doubleClick(w, {120, 300});
    onNextDialog([](QDialog *d) { fillAndOk(d, "0", "func1"); });
    doubleClick(w, {400, 300});
    onNextDialog([](QDialog *d) { fillAndOk(d, "1"); });
    doubleClick(w, mid(w->tp[0]));
    onNextDialog([](QDialog *d) { fillAndOk(d, "1"); });
    doubleClick(w, mid(w->tp[1]));
    w->ui->actionrunc->trigger();
    shot(w, "r9_self_loop");
    report("R9", "允许连自环；运行时这个神经元算不出来，下游全是 0，没有提示",
           w->tp.size() == 3 && w->Oneure[0].output() == 0,
           "连自环时直接拒绝；输出 = 1",
           QString("边 %1 都被接受；运行后隐层显示输出 %2，输出神经元却是 %3")
               .arg(edgesGui(w)).arg(w->Hneure[0].Output()).arg(w->Oneure[0].output()));
    delete w;
}

// R10 线的命中判断假设 x1 < x2：从右往左的线点不中，删不掉也设不了权值
static void r10()
{
    MainWindow *w = newWindow();
    w->ui->actionIneure->trigger();   // 0
    w->ui->actionHneure->trigger();   // 1
    w->Ineure[0].setpos(550, 250);
    w->Hneure[0].setpos(200, 250);
    rightDrag(w, {550, 250}, {200, 250});
    w->ui->actiondeletewire->trigger();
    leftClick(w, {370, 250});
    shot(w, "r10_right_to_left_wire");
    report("R10", "从右往左连的线点不中（删不掉、也设不了权值）", w->tp.size() == 1,
           "点在线上就能删掉",
           QString("点了线的正中间，线还在：%1。线是从输入神经元右边缘画到隐层左边缘，横穿两个圆")
               .arg(edgesGui(w)));
    delete w;
}

// R11 两个不同层的神经元重叠后，一拖就一起走，再也分不开
static void r11()
{
    MainWindow *w = newWindow();
    w->ui->actionIneure->trigger();
    w->ui->actionHneure->trigger();
    w->Ineure[0].setpos(300, 300);
    w->Hneure[0].setpos(300, 300);
    leftDrag(w, {300, 300}, {300, 460}, 16);
    QPoint pi(w->Ineure[0].getx(), w->Ineure[0].gety()), ph(w->Hneure[0].getX(), w->Hneure[0].getY());
    report("R11", "输入和隐层神经元叠在一起后，拖一个两个一起动", pi == ph && pi == QPoint(300, 460),
           "只拖走最上面的那一个",
           QString("输入神经元到了 (%1,%2)，隐层神经元也到了 (%3,%4)")
               .arg(pi.x()).arg(pi.y()).arg(ph.x()).arg(ph.y()));
    delete w;
}

// R12 鼠标一次移动超过半径（50px），就拖不动了
static void r12()
{
    MainWindow *w = newWindow();
    w->ui->actionIneure->trigger();   // 默认在 (100,100)
    leftDrag(w, {100, 100}, {100, 260}, 1);
    report("R12", "鼠标移动快一点（一次超过 50px）神经元就拖不动", w->Ineure[0].gety() == 100,
           "神经元跟到 (100,260)",
           QString("神经元还在 (%1,%2)：移动事件里重新判断“鼠标在不在圆里”，鼠标先出了圆就不认了")
               .arg(w->Ineure[0].getx()).arg(w->Ineure[0].gety()));
    delete w;
}

// R13 每次双击都 new 一个对话框，关掉也不释放（父对象是主窗口，主窗口关了才删）
static void r13()
{
    MainWindow *w = newWindow();
    w->ui->actionIneure->trigger();
    w->Ineure[0].setpos(200, 200);
    for (int i = 0; i < 5; ++i) {
        onNextDialog([](QDialog *d) { pressCancel(d); });
        doubleClick(w, {200, 200});
    }
    int n = w->findChildren<QDialog *>().size();
    report("R13", "每次双击 new 一个对话框，关掉后不释放", n >= 5, "对话框关掉就释放，剩 0 个",
           QString("双击 5 次后，还有 %1 个对话框对象活着").arg(n));
    delete w;
}

// R14 “删除线”“删除点”共用一个 deleteallkind：选了删除线，点到神经元也删；点空白处模式一直挂着
static void r14()
{
    MainWindow *w = newWindow();
    w->ui->actionIneure->trigger();   // 0，默认 (100,100)
    w->ui->actionOneure->trigger();   // 1，默认 (500,100)
    w->ui->actiondeletewire->trigger();
    leftClick(w, {300, 400});          // 点了空白处
    const int armed = w->deleteallkind;
    leftClick(w, {100, 100});          // 之后想拖动输入神经元
    report("R14", "选了「删除线」，点神经元也会把神经元删掉；点空白处删除模式一直挂着",
           armed == 1 && w->Ineure.empty(),
           "「删除线」只删线；没点中目标时给提示，可以取消",
           QString("点空白后 deleteallkind 仍是 %1（还在待命）；接着点输入神经元，它被删了（剩 %2 个输入神经元）")
               .arg(armed).arg(w->Ineure.size()));
    delete w;
}

// R15 删掉线再运行：没有线进来的神经元不会被重新计算，还显示上一次的结果
static void r15()
{
    MainWindow *w = newWindow();
    w->ui->actionIneure->trigger();
    w->ui->actionOneure->trigger();
    w->Ineure[0].setpos(150, 300);
    w->Oneure[0].setpos(550, 300);
    rightDrag(w, {150, 300}, {550, 300});
    onNextDialog([](QDialog *d) { fillAndOk(d, "1"); });
    doubleClick(w, {150, 300});
    onNextDialog([](QDialog *d) { fillAndOk(d, "2"); });
    doubleClick(w, {350, 300});
    w->ui->actionrunc->trigger();
    const float first = w->Oneure[0].output();
    w->ui->actiondeletewire->trigger();
    leftClick(w, {350, 300});
    w->ui->actionrunc->trigger();
    const float second = w->Oneure[0].output();
    report("R15", "删掉线再运行，输出还是上一次的结果", w->tp.empty() && std::fabs(second - 2) < 1e-6f,
           "线删了，输出应变为 0",
           QString("删线前输出 %1；删线后（界面上已没有线）再运行，输出仍是 %2").arg(first).arg(second));
    delete w;
}

// B 基准：同一个分层全连接网络（没有跳层连接，原版在这种网络上结果是对的），比较一次运行的耗时
static QStringList g_bench;
static void bench(int nIn, int nHid, int nOut)
{
    // ---- 原版 Adjacency_List::TopSort ----
    Adjacency_List g;
    int key = 0;
    std::vector<int> ins, hids, outs;
    for (int k = 0; k < nIn; ++k) { g.addvertax(key); g.addInput(InputNeure(key)); ins.push_back(key++); }
    for (int k = 0; k < nHid; ++k) {
        g.addvertax(key);
        hidden_layer h(key); h.setjiflag(1); h.setthr(0);
        g.addhidden(h);
        hids.push_back(key++);
    }
    for (int k = 0; k < nOut; ++k) { g.addvertax(key); g.addOutput(OutputNeure(key)); outs.push_back(key++); }
    std::vector<transport> tv;
    int ek = 0;
    auto link = [&](int a, int b) {
        transport t; t.setfrom(a); t.setto(b); g.addEdge(t);
        transport w; w.setkey(ek++); w.setWeight(0.01f); tv.push_back(w);
    };
    for (int a : ins) for (int b : hids) link(a, b);
    for (int a : hids) for (int b : outs) link(a, b);
    for (size_t i = 0; i < tv.size(); ++i) g.intransport(tv.begin() + i);
    std::vector<InputNeure> iv;
    for (int k : ins) { InputNeure x(k); x.setInput(1); iv.push_back(x); }
    for (size_t i = 0; i < iv.size(); ++i) g.inInput(iv.begin() + i);
    QElapsedTimer t;
    t.start();
    g.TopSort();
    const double oldMs = t.nsecsElapsed() / 1e6;
    const float oldOut = g.Outne[0].output();

    // ---- 新版 Network::run ----
    Network n;
    std::vector<int> a2, b2, c2;
    for (int k = 0; k < nIn; ++k) a2.push_back(n.addNeuron(NeuronKind::Input, {}));
    for (int k = 0; k < nHid; ++k) b2.push_back(n.addNeuron(NeuronKind::Hidden, {}));
    for (int k = 0; k < nOut; ++k) c2.push_back(n.addNeuron(NeuronKind::Output, {}));
    for (int x : a2) for (int y : b2) n.setWeight(n.addEdge(x, y), 0.01);
    for (int x : b2) for (int y : c2) n.setWeight(n.addEdge(x, y), 0.01);
    for (int x : a2) n.setInputValue(x, 1);
    const int rounds = 200;
    t.restart();
    for (int r = 0; r < rounds; ++r) n.run();
    const double newMs = t.nsecsElapsed() / 1e6 / rounds;
    const double newOut = n.neuron(c2[0])->output;

    g_bench << QString("%1-%2-%3 全连接（%4 个神经元，%5 条边）：原版 TopSort %6 ms，新版 run() %7 ms（%8 倍）；"
                       "输出 原版 %9 / 新版 %10")
                   .arg(nIn).arg(nHid).arg(nOut).arg(nIn + nHid + nOut).arg(tv.size())
                   .arg(oldMs, 0, 'f', 2).arg(newMs, 0, 'f', 4).arg(oldMs / newMs, 0, 'f', 0)
                   .arg(oldOut).arg(newOut);
}

static void quietHandler(QtMsgType t, const QMessageLogContext &, const QString &m)
{
    if (t != QtDebugMsg) std::cerr << m.toLocal8Bit().constData() << "\n";   // 原版 qDebug 太多，只留警告
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QApplication::setFont(QFont("Microsoft YaHei UI", 9));   // offscreen 下统一字体，截图接近真实 Windows 界面
    qInstallMessageHandler(quietHandler);
    QStringList args = app.arguments();
    g_out = args.size() > 1 ? args.last() : QDir::currentPath();
    QDir().mkpath(g_out + "/shots");

    r1(); r2(); r3(); r3b(); r4(); r4gui(); r5(); r6(); r7(); r8(); r9(); r10(); r11(); r12(); r13(); r14(); r15();
    bench(4, 4, 2);    // 平时手搭的规模
    bench(10, 10, 4);
    bench(30, 30, 10);
    bench(60, 60, 20);

    g_report.prepend(QString("原版（git HEAD 4ffc9e2）复现结果：%1 / %2 条复现\n").arg(g_reproduced).arg(g_total));
    g_report << "[基准] 同一网络跑一次前向计算的耗时（原版编译时关掉了 qDebug，只比算法）" << g_bench;
    QFile f(g_out + "/repro_report.txt");
    f.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream ts(&f);
    ts.setEncoding(QStringConverter::Utf8);
    ts << g_report.join("\n") << "\n";
    std::cout << "done " << g_reproduced << "/" << g_total << std::endl;
    return 0;
}
