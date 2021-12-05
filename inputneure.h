#ifndef INPUTNEURE_H
#define INPUTNEURE_H
#include "neure.h"


class InputNeure:Neure
{
private:
     float in=0,out=0;
     int  key;          //神经元标号
     int posx=100,posy=100;     //原点坐标
     int flag;      //鼠标拖动

     string kind="Inputneure";
public:
  InputNeure();
  InputNeure(int n);
  void setInput(float in);
  float getInput();
  float getoutput();
  int getx();
  int gety();
  void setkey(int key);
  int getkey();
  void setflag(int flag);
  int getflag();
  void setpos(int posx,int posy);
  void setOutput();



};

#endif // INPUTNEURE_H
