#ifndef OUTPUTNEURE_H
#define OUTPUTNEURE_H
#include "neure.h"

 class OutputNeure:Neure
{
 private:
     float in=0,out=0;
     int key;
     int flag;
     int posx=500,posy=100;
     string m="Outputneure";
public:
    OutputNeure();
    OutputNeure(int n);
    float input();
    float output();
    int getX();
    int getY();
    int getflag();
    void setflag(int flag);
    void setkey(int key);
    int getkey();
    void setin(float in);
    void setout();
    void setpos(int posx,int posy);

   // void Activation_Function(int flag); //激活函数
};

#endif // OUTPUTNEURE_H
