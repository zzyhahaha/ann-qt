#ifndef HIDDEN_LAYER_H
#define HIDDEN_LAYER_H
#include"neure.h"

class hidden_layer
{
private:
    float in=0,out=0;
    int key;
    int flag; //鼠标拖动
    int posx=300,posy=100;
    float threshold=0; //阈值
    int jiflag; //哪个激活函数
public:
    hidden_layer();
    hidden_layer(int n);
    float Input();
    float Output();
    void setkey(int key);
    int getkey();
    int getX();
    int getY();
    int getflag();//防拖动出bug
    float getthr(); //获取阈值
    void setflag(int flag);
    void setin(float in);
    void setout(float point);
    void setpos(int posx,int posy);
    void setthr(float thr);//设置阈值
    float Activation_Function(int flag);//激活函数
    void setjiflag(int flag);
    int  getjiflag();
};

#endif // HIDDEN_LAYER_H
