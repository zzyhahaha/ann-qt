#ifndef INPUTNEURE_H
#define INPUTNEURE_H


class InputNeure
{
private:
     float in,out;
     int  key;
public:
     InputNeure();
     InputNeure(int n);
  float input(float in);
  float output(float out);
};

#endif // INPUTNEURE_H
