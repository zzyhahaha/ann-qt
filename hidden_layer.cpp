#include "hidden_layer.h"

hidden_layer:: hidden_layer(){
}
hidden_layer:: hidden_layer(int n)
{
    key=n;
}
float hidden_layer::Input(){
    return this->in;
}
float hidden_layer::Output(){
    return this->out;
}
void hidden_layer::setkey(int key){
    this->key=key;
}
int hidden_layer::getkey(){
    return this->key;
}
void hidden_layer::setpos(int posx,int posy){
    this->posx=posx;
    this->posy=posy;
}
int hidden_layer::getX(){
    return this->posx;
}
int hidden_layer::getY(){
    return this->posy;
}
void hidden_layer::setflag(int flag){
    this->flag=flag;
}
int hidden_layer::getflag(){
    return this->flag;
}
float hidden_layer::getthr(){
    return this->threshold;
}

void hidden_layer::setthr(float thr)
{
    this->threshold=thr;
}

float hidden_layer::Activation_Function(int flag){
    if(flag==1){                            //  y=x
        if(in<=1 && in>=-1){
            out=in-this->threshold;
        }
        else if(in>1)
            out=1-this->threshold;
        else
            out=-1-this->threshold;

    }
    else if(flag==2){
                                  //y=0或y=1
        if(in>0){
            out=1-this->threshold;
        }
        else
            out=0-this->threshold;
    }
    return out;
}

void hidden_layer::setjiflag(int flag){
    this->jiflag=flag;

}
int hidden_layer::getjiflag(){
    return jiflag;
}
void hidden_layer::setin(float in){
    this->in=in;
}
void hidden_layer::setout(float point){
    this->out=point;
}

