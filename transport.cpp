#include "transport.h"
transport::transport(){
    in=0;
    out=0;
}
 transport::transport (float in){
     this->in=in;
}
 float transport::output(float weight){
     out=weight*this->in;
     return out;
 }
 float transport::input(float in){
     this->in=in;
     return this->in;
 }
 float transport::getWeight(){
     return this->weight;
 }
 int transport::getfrom(){
     return this->from;
 }
 int transport::getto(){
     return this->to;
 }
 int transport::getkey(){
     return this->key;
 }
 void transport::setWeight(float weight){
     this->weight=weight;
 }
 void transport::setfrom(int key){
     this->from=key;
 }
 void transport::setto(int key){
     this->to=key;
 }
 int transport::getposx1(){
     return this->posx1;
 }
 int transport::getposy1(){
     return this->posy1;
 }
 int transport::getposx2(){
     return this->posx2;
 }
 int transport::getposy2(){
     return this->posy2;
 }
 void transport::SetposFrom(int posx1, int posy1){
     this->posx1=posx1;
     this->posy1=posy1;

 }
 void transport::SetposTo(int posx2, int posy2){
     this->posx2=posx2;
     this->posy2=posy2;
 }
 void transport::setkey(int key){
     this->key=key;
 }








