#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <ui_dialog.h>
#include <QDebug>
#include <vector>
#include <QPushButton>
#include <QPainter>
#include <QAction>
#include <QToolBar>
#include <QFile>
#include <iostream>
using namespace std;


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    resize(800,600);

//新建输入层神经元
    connect(ui->actionIneure,&QAction::triggered,this,[=](){
        InputNeure temp;
        temp.setkey(key);
        Ineure.push_back(temp);
        a.addvertax(key);
        a.addInput(temp);
        qDebug()<<key<<a.getvertax();
        key++;



   });

//新建隐层神经元
    connect(ui->actionHneure,&QAction::triggered,this,[=](){
        hidden_layer temp;
        temp.setkey(key);
        Hneure.push_back(key);
        a.addvertax(key);
        a.addhidden(temp);
        qDebug()<<key<<a.getvertax();
        key++;




   });

//新建输出层神经元
    connect(ui->actionOneure,&QAction::triggered,this,[=](){
        OutputNeure temp;
        temp.setkey(key);
        Oneure.push_back(temp);
        a.addvertax(key);
        a.addOutput(temp);
        qDebug()<<key<<a.getvertax();
        key++;



   });
//清空界面
    connect(ui->actionclr,&QAction::triggered,this,[=](){
        key=0;
        Ineure.clear();
        Hneure.clear();
        Oneure.clear();
        tp.clear();
        a.clrdesk();
        update();
   });
//运行
    connect(ui->actionrunc,&QAction::triggered,this,[=](){
       a.TopSort();
       for(vector<InputNeure>::iterator it=this->Ineure.begin();it!=this->Ineure.end();){
           a.retInput(it);
           it++;
       }
       for(vector<hidden_layer>::iterator it=this->Hneure.begin();it!=this->Hneure.end();){
           a.retHidden(it);
           it++;
       }

        for(vector<OutputNeure>::iterator it=this->Oneure.begin();it!=this->Oneure.end();){
            a.retOutput(it);
            it++;
        }

    });

    connect(ui->actiondeletewire,&QAction::triggered,this,[=](){

        deleteallkind=1;

    });//删除线


    connect(ui->actiondeletenode,&QAction::triggered,this,[=](){

        deleteallkind=1;

    });//删除节点


    connect(ui->actionprintdata,&QAction::triggered,this,[=](){

        QFile f("C:\\Users\\67495\\Desktop\\test.txt");

        if(!f.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            cout << "Open failed." << endl;

        }
        QTextStream txtOutput(&f);
        QString s1("Inputneure:");
        txtOutput<<'\n';
        txtOutput<<s1;
        txtOutput<<"\n";
        for(int i=0;i<(int)Ineure.size();i++){

            txtOutput<<"神经元编号: "<<Ineure[i].getkey()<<"  输入: "<<Ineure[i].getInput()<<"  输出: "<<Ineure[i].getoutput();
            txtOutput<<'\n';
        }
        txtOutput<<'\n';
        QString s2("hidden_layer:");
        txtOutput<<s2;
        txtOutput<<"\n";
        for(int i=0;i<(int)Hneure.size();i++){
            txtOutput<<"神经元编号: "<<Hneure[i].getkey()<<"  输入: "<<Hneure[i].Input()<<"  输出: "<<Hneure[i].Output()<<"  阈值: "<<Hneure[i].getthr()<<"  激活函数编号: "<<Hneure[i].getjiflag();
            txtOutput<<'\n';
        }

        QString s3("OutputNeure:");
        txtOutput<<'\n';
        txtOutput<<s3;
        txtOutput<<'\n';
        for(int i=0;i<(int)Oneure.size();i++){

            txtOutput<<"神经元编号: "<<Oneure[i].getkey()<<"  输入: "<<Oneure[i].input()<<"  输出: "<<Oneure[i].output();
            txtOutput<<'\n';
        }
        txtOutput<<'\n';
        QString s4("wire:");
        txtOutput<<s4;
        txtOutput<<'\n';
        for(int i=0;i<(int)tp.size();i++){

            txtOutput<<"边的编号: "<<tp[i].getkey()<<"  边的权重: "<<tp[i].getWeight()<<"  from 神经元标号: "<<tp[i].getfrom()<<"  to 神经元标号: "<<tp[i].getto();
            txtOutput<<"\n";
        }
        f.close();

    });//打印数据




}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::paintEvent(QPaintEvent *){
////指定绘图的设备
    drawNeure();
    drawTran();
    //drawtmp(tmp);
}
void MainWindow::drawtmp(transport tmp){
    QPainter painter(this);
    QPen pen(QColor(0,0,0));
    pen.setWidth(2);
    painter.setPen(pen);
    painter.drawLine(QPoint(tmp.getposx1(),tmp.getposy1()),QPoint(tmp.getposx2(),tmp.getposy2()));
    update();
}



void MainWindow::drawNeure(){
    QPainter painter(this);
    QPen pen(QColor(255,0,0));
    pen.setWidth(3);
    //QFont font1("宋体",15,QFont::Bold,true);
    painter.setPen(pen);
    /*输入神经元*/
    for(vector<InputNeure>::iterator it=Ineure.begin();it!=Ineure.end();){
        QString output,key,input;
        key=QString::number(it->getkey());
        input=QString("%1").arg(it->getInput());
        output=QString("%1").arg(it->getoutput());
        painter.drawEllipse(QPoint(it->getx(),it->gety()),50,50);
        painter.drawText(QPoint(it->getx()-30,it->gety()+30),tr("总输出:")+output);
        painter.drawText(QPoint(it->getx()-30,it->gety()+10),tr("Key:")+key);
        painter.drawText(QPoint(it->getx()-30,it->gety()-10),tr("总输入:")+input);
        it++;
    }

    pen.setColor(QColor(0,255,0));
    painter.setPen(pen);
    /*隐层神经元*/
    for(vector<hidden_layer>::iterator it=Hneure.begin();it!=Hneure.end();){
        painter.drawEllipse(QPoint(it->getX(),it->getY()),60,60);
        QString flag,output,key,input,thr;
        flag=QString::number(it->getjiflag());
        key=QString::number(it->getkey());
        input=QString("%1").arg(it->Input());
        output=QString("%1").arg(it->Output());
        thr=QString("%1").arg(it->getthr());
        painter.drawText(QPoint(it->getX()-35,it->getY()-20),tr("激活函数:")+flag);
        painter.drawText(QPoint(it->getX()-35,it->getY()+40),tr("总输出:")+output);
        painter.drawText(QPoint(it->getX()-35,it->getY()+20),tr("Key:")+key);
        painter.drawText(QPoint(it->getX()-35,it->getY()),tr("总输入:")+input);
        painter.drawText(QPoint(it->getX()-35,it->getY()-40),tr("阈值:")+thr);
        it++;
    }

    pen.setColor(QColor(0,0,255));
    painter.setPen(pen);
    /*输出神经元*/
    for(vector<OutputNeure>::iterator it=Oneure.begin();it!=Oneure.end();){
        QString output,key,input;
        key=QString::number(it->getkey());
        input=QString("%1").arg(it->input());
        output=QString("%1").arg(it->output());
        painter.drawEllipse(QPoint(it->getX(),it->getY()),50,50);
        painter.drawText(QPoint(it->getX()-30,it->getY()+30),tr("总输出:")+output);
        painter.drawText(QPoint(it->getX()-30,it->getY()+10),tr("Key:")+key);
        painter.drawText(QPoint(it->getX()-30,it->getY()-10),tr("总输入:")+input);
        it++;
    }
    update();

}

void MainWindow::drawTran(){
     QPainter painter(this);
     QPen pen(QColor(0,0,0),2);

     painter.setPen(pen);
     for(vector<transport>::iterator it=tp.begin();it!=tp.end();){
          painter.drawLine(it->getposx1(),it->getposy1(),it->getposx2(),it->getposy2());
          QString weight;
          weight=QString("%1").arg(it->getWeight());
          painter.drawText(QPoint(((it->getposx1()+it->getposx2())/2)-10,((it->getposy1()+it->getposy2())/2)-5),tr("weight:")+weight);
          it++;
     }
     update();

}

void MainWindow::mouseMoveEvent(QMouseEvent *ev){
    if(ev->buttons() & Qt::LeftButton){
        //输入端神经元位置改变
        for(vector<InputNeure>::iterator it=Ineure.begin();it!=Ineure.end();){
            if(ev->pos().x()>it->getx()-50 & ev->pos().x()<it->getx()+50 & ev->pos().y()<it->gety()+50 & ev->pos().y()>it->gety()-50 & it->getflag()==1 ){
                it->setpos(ev->pos().x(),ev->pos().y());

                    for(vector<transport>::iterator iter=tp.begin();iter!=tp.end();)
                {
                 if(iter->getfrom()==it->getkey()){
                       iter->SetposFrom(it->getx()+50,it->gety());
                    }
                    iter++;
                }
                break;
            }
            it++;
        }
        //隐层神经元位置改变
        for(vector<hidden_layer>::iterator it=Hneure.begin();it!=Hneure.end();){

            if(ev->pos().x()>it->getX()-60 & ev->pos().x()<it->getX()+60 & ev->pos().y()<it->getY()+60 &ev->pos().y()>it->getY()-60 & it->getflag()==1){
                it->setpos(ev->pos().x(),ev->pos().y());
                for(vector<transport>::iterator iter=tp.begin();iter!=tp.end();)
                {
                    if(tp.size()==0)
                        break;
                    // qDebug()<<iter->getfrom()<<iter->getto();
                    if(iter->getfrom()==it->getkey()){
                        iter->SetposFrom(it->getX()+60,it->getY());
                    }
                     if(iter->getto()==it->getkey()){
                        iter->SetposTo(it->getX()-60,it->getY());
                    }
                        iter++;
                }
                break;
            }
            it++;
        }
        //输出层神经元位置改变
        for(vector<OutputNeure>::iterator it=Oneure.begin();it!=Oneure.end();){
            if(ev->pos().x()>it->getX()-50 & ev->pos().x()<it->getX()+50 & ev->pos().y()<it->getY()+50 &ev->pos().y()>it->getY()-50 & it->getflag()==1){
                it->setpos(ev->pos().x(),ev->pos().y());
                for(vector<transport>::iterator iter=tp.begin();iter!=tp.end();)
                {
                    if(tp.size()==0)
                        break;

                    if(iter->getto()==it->getkey()){
                        iter->SetposTo(it->getX()-50,it->getY());
                    }
                    iter++;
                }
            break;
            }
            it++;
        }

    }
/*****************************线的位置*****************************/
    else if(ev->button() & Qt::RightButton){
            tmp.SetposTo(ev->pos().x(),ev->pos().y());
        }

}



void MainWindow::mousePressEvent(QMouseEvent *ev){
    if(ev->buttons() == Qt::LeftButton){
        //输入端神经元位置改变
        for(int i=0;i<(int)Ineure.size();i++){
            if(Ineure.size()==0)
                break;

            else{
                if(ev->pos().x()>Ineure[i].getx()-50 & ev->pos().x()<Ineure[i].getx()+50 & ev->pos().y()<Ineure[i].gety()+50 & ev->pos().y()>Ineure[i].gety()-50){
                    Ineure[i].setflag(1);
                    if(deleteallkind==1)
                    {
                        a.delvertax(Ineure[i].getkey());
                        Ineure.erase(Ineure.begin()+i);
                        delInputneure(Ineure[i]);
                        deleteallkind=0;
                    }
                    break;
            }
                else{
                    Ineure[i].setflag(0);

                }
           }
}
       for(int i=0;i<(int)Hneure.size();i++){
           if(Hneure.size()==0)
               break;
           else{
                if(ev->pos().x()>Hneure[i].getX()-50 & ev->pos().x()<Hneure[i].getX()+50 & ev->pos().y()<Hneure[i].getY()+50 &ev->pos().y()>Hneure[i].getY()-50){
                    Hneure[i].setflag(1);
                    if(deleteallkind==1)
                    {
                        a.delvertax(Hneure[i].getkey());
                        Hneure.erase(Hneure.begin()+i);
                        delhiddenneure(Hneure[i]);
                        deleteallkind=0;
                    }
                    break;
            }
                else{
                    Hneure[i].setflag(0);
            }
          }
}
       for(int i=0;i<(int)Oneure.size();i++)   {
           if(Oneure.size()==0)
               break;
           else{
                if(ev->pos().x()>Oneure[i].getX()-50 & ev->pos().x()<Oneure[i].getX()+50 & ev->pos().y()<Oneure[i].getY()+50 &ev->pos().y()>Oneure[i].getY()-50){
                Oneure[i].setflag(1);

                if(deleteallkind==1)
                {
                    a.delvertax(Oneure[i].getkey());
                    Oneure.erase(Oneure.begin()+i);
                    delOutputneure(Oneure[i]);
                    deleteallkind=0;
                }

                break;
            }
                else{
                    Oneure[i].setflag(0);

                }

           }
        }
/***************************************************删除线**************************************************/
       for(int i=0;i<(int)tp.size();i++){
           if(tp.size()==0)
               break;
           else{
               if(ev->pos().x()>tp[i].getposx1() & ev->pos().x()<tp[i].getposx2()){
                   float k=0.0,b=0.0;
                   k=(tp[i].getposy1()-tp[i].getposy2())/(tp[i].getposx1()-tp[i].getposx2()*1.0);
                   b=(tp[i].getposx2()*tp[i].getposy1()-tp[i].getposx1()*tp[i].getposy2())/(tp[i].getposx2()-tp[i].getposx1()*1.0);
                   if(abs(ev->pos().y()-k*ev->pos().x()-b)<3 && deleteallkind==1){
                            a.delEdge(tp[i]);
                            deleteallkind=0;
                            tp.erase(tp.begin()+i);
                      }
                  }
             }
       }

}
    if(ev->button()==Qt::RightButton){
        //输入层线的起始位置
        for(vector<InputNeure>::iterator it=Ineure.begin();it!=Ineure.end();){
            if(ev->pos().x()>it->getx()-50 & ev->pos().x()<it->getx()+50 & ev->pos().y()<it->gety()+50 & ev->pos().y()>it->gety()-50){
                tmp.setfrom(it->getkey());
                tmp.SetposFrom(it->getx()+50,it->gety());
            }
            it++;
        }
        //隐层线的起始位置
        for(vector<hidden_layer>::iterator it=Hneure.begin();it!=Hneure.end();){
            if(ev->pos().x()>it->getX()-60 & ev->pos().x()<it->getX()+60 & ev->pos().y()<it->getY()+60 & ev->pos().y()>it->getY()-60){
                tmp.setfrom(it->getkey());
                tmp.SetposFrom(it->getX()+60,it->getY());
            }
            it++;
        }

    }

}

void MainWindow::mouseReleaseEvent(QMouseEvent *ev){
    if(ev->buttons() == Qt::LeftButton){
        //输入端神经元位置改变
        for(vector<InputNeure>::iterator it=Ineure.begin();it!=Ineure.end();){
                if(Ineure.size()==0)
                break;
                it->setflag(0);
                it++;
            }


       for(vector<hidden_layer>::iterator it=Hneure.begin();it!=Hneure.end();){
                if(Hneure.size()==0)
                break;
                it->setflag(0);
                it++;
            }

       for(vector<OutputNeure>::iterator it=Oneure.begin();it!=Oneure.end();){
                if(Oneure.size()==0)
                break;
                it->setflag(0);
                it++;
            }

}




    if(ev->button()==Qt::RightButton){
        //隐层线的终止位置
        for(vector<hidden_layer>::iterator it=Hneure.begin();it!=Hneure.end();){
            if(ev->pos().x()>it->getX()-60 & ev->pos().x()<it->getX()+60 & ev->pos().y()<it->getY()+60 &ev->pos().y()>it->getY()-60){
                tmp.setto(it->getkey());
                tmp.SetposTo(it->getX()-60,it->getY());
                qDebug()<<tmp.getfrom()<<tmp.getto();
                tmp.setkey(transkey);
                transkey++;
                tp.push_back(tmp);
                a.addEdge(tmp);
            }
            it++;
        }
        //输出层线的终止位置
        for(vector<OutputNeure>::iterator it=Oneure.begin();it!=Oneure.end();){
            if(ev->pos().x()>it->getX()-50 & ev->pos().x()<it->getX()+50 &ev->pos().y()<it->getY()+50 & ev->pos().y()>it->getY()-50){
               tmp.setto(it->getkey());
               tmp.SetposTo(it->getX()-50,it->getY());
               qDebug()<<tmp.getfrom()<<tmp.getto();
               tmp.setkey(transkey);
               transkey++;
               tp.push_back(tmp);
               a.addEdge(tmp);
            }
            it++;
        }

    }

}


void MainWindow::mouseDoubleClickEvent(QMouseEvent *ev){
    if(ev->button()==Qt::LeftButton){
        for(vector<InputNeure>::iterator it=Ineure.begin();it!=Ineure.end();){
            if(Ineure.size()==0)
                break;
            if(ev->pos().x()>it->getx()-50 & ev->pos().x()<it->getx()+50 & ev->pos().y()<it->gety()+50 & ev->pos().y()>it->gety()-50){
            //生成对话框---对话框3
                 Dialog3 *dl3= new Dialog3(this);
                 dl3->exec();
                 QString s3;
                 s3=dl3->getinput();
                 float in=0;
                 in=s3.toFloat();
                 it->setInput(in);
                 a.inInput(it);


            }
            it++;
        }
        for(vector<hidden_layer>::iterator it=Hneure.begin();it!=Hneure.end();){
                if(Hneure.size()==0)
                    break;

            if(ev->pos().x()>it->getX()-50 & ev->pos().x()<it->getX()+50 & ev->pos().y()<it->getY()+50 &ev->pos().y()>it->getY()-50){
             //显示激活函数---对话框1
                Dialog *dl=new Dialog(this);
                dl->exec();
                /*获取文本内容*/
                QString Str;
                Str=dl->getinput();

                float threshold;
                threshold=Str.toFloat();
                it->setthr(threshold);
                qDebug()<<it->getthr()<<Str;
                int Aflag=dl->getflag();
                it->setjiflag(Aflag);
                a.inhidden(it);
                qDebug()<<Aflag;

            }
            it++;
        }

      for(vector<transport>::iterator it=tp.begin();it!=tp.end();){
         if(tp.size()==0)
                break;
         if(ev->pos().x()>it->getposx1() & ev->pos().x()<it->getposx2()){
             float k=0.0,b=0.0;
             k=(it->getposy1()-it->getposy2())/(it->getposx1()-it->getposx2()*1.0);
             b=(it->getposx2()*it->getposy1()-it->getposx1()*it->getposy2())/(it->getposx2()-it->getposx1()*1.0);
                    //显示设置权值----对话框2
             if(abs(ev->pos().y()-k*ev->pos().x()-b)<3){
                    qDebug()<<"keyishele";
                    Dialog2 *dl2=new Dialog2(this);
                    dl2->exec();
                    QString s;
                    s=dl2->getinput();
                    float inweight=0;
                    inweight=s.toFloat();
                    it->setWeight(inweight);
                    a.intransport(it);
                    qDebug()<<"setWeight:"<<inweight;
                }           
            }
            it++;
        }

    }

}

void MainWindow::delInputneure(InputNeure Ineure1){
    for(int i=0;i<(int)tp.size();i++){
        if(Ineure1.getkey()==tp[i].getfrom()){
            tp.erase(tp.begin()+i);
            i=0;
        }
    }
}

void MainWindow::delhiddenneure(hidden_layer Hneure1){
    for(int i=0;i<(int)tp.size();i++){
        if(Hneure1.getkey()==tp[i].getfrom()){
            tp.erase(tp.begin()+i);
            qDebug()<<"tp-erase:"<<tp[i].getkey()<<tp[i].getfrom()<<tp[i].getto();
            i=0;
        }
    }
    for(int i=0;i<(int)tp.size();i++){
        if(Hneure1.getkey()==tp[i].getto()){
            tp.erase(tp.begin()+i);
            qDebug()<<"tp-erase:"<<tp[i].getkey()<<tp[i].getfrom()<<tp[i].getto();
            i=0;
        }
    }

}

void MainWindow::delOutputneure(OutputNeure Oneure1){
    for(int i=0;i<(int)tp.size();i++){
        if(Oneure1.getkey()==tp[i].getto()){
            tp.erase(tp.begin()+i);
            i=0;
        }
    }

}







