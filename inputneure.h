#ifndef INPUTNEURE_H
#define INPUTNEURE_H
#include "neure.h"

class InputNeure:Neure
{
private:
     float in,out;
     int  key;          //神经元标号
     int posx,posy;     //原点坐标
     string kind="Inputneure";
public:
  InputNeure();
  InputNeure(int n);
  void setInput(float in);
  float getInput();

  float output(float out);
  int getx();
  int gety();


};

#endif // INPUTNEURE_H
