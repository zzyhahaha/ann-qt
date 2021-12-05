#include "outputneure.h"
OutputNeure::OutputNeure(){

}
OutputNeure::OutputNeure(int n)
{
    this->key=n;
}
float OutputNeure::input(){
    return this->in;
}
float OutputNeure::output(){
    return this->out;
}
int OutputNeure::getX(){
    return this->posx;
}
int OutputNeure::getY(){
    return this->posy;
}

void OutputNeure::setkey(int key){
    this->key=key;

}
int OutputNeure::getkey(){
    return this->key;
}
void OutputNeure::setpos(int posx, int posy){
    this->posx=posx;
    this->posy=posy;
}
void OutputNeure::setin(float in){
    this->in=in;
}
void OutputNeure::setout(){
    this->out=this->in;
}
int OutputNeure::getflag(){
    return this->flag;
}
void OutputNeure::setflag(int flag){
    this->flag=flag;
}
