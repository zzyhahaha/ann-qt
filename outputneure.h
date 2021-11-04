#ifndef OUTPUTNEURE_H
#define OUTPUTNEURE_H
#include "neure.h"

 class OutputNeure:Neure
{
 private:
     float in,out;
     int key;
     int posx,posy;
     string m="Outputneure";
public:
    OutputNeure();
    OutputNeure(int n);
    float input();
    float output();
    void Activation_Function(int flag); //激活函数
};

#endif // OUTPUTNEURE_H
