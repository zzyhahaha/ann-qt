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

float InputNeure::getoutput(){

    return this->out;
}
int InputNeure:: getx(){
    return posx;
}
int InputNeure::gety(){
    return posy;
}
void InputNeure::setkey(int key){
   this->key=key;
}
int InputNeure::getkey(){
    return this->key;
}
int InputNeure::getflag(){
    return this->flag;
}

void InputNeure::setflag(int flag){
    this->flag=flag;

}
void InputNeure::setpos(int posx, int posy){
    this->posx=posx;
    this->posy=posy;

}
void InputNeure::setOutput(){
    this->out=this->in;
}
