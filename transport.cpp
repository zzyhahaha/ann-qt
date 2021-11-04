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

