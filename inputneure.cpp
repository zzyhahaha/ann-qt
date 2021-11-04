#include "inputneure.h"
#include <iostream>
using namespace std;
InputNeure::InputNeure():Neure(){
    key=0;
    in=0;
    posx=posy=100;
    this->out=this->in;
}
InputNeure::InputNeure(int n):Neure(n)
{
    key=n;
    in=0;
    posx=posy=100;
    this->out=this->in;
}
float InputNeure::getInput()
{
    return this->in;
}
void InputNeure::setInput(float in){
    this->in=in;
}
float InputNeure::output(float out){
    this->out=this->in;
    return this->out;
}
int InputNeure:: getx(){
    return posx;
}
int InputNeure::gety(){
    return posy;
}
