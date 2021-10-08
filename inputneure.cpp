#include "inputneure.h"
#include <iostream>
using namespace std;
InputNeure::InputNeure(){
    key=0;
    in=0;
}
InputNeure::InputNeure(int n)
{
    key=n;
    in=0;
}
float InputNeure::input(float in)
{
    this->in=in;
    return this->in;
}
float InputNeure::output(float out){
    this->out=this->in;
    return this->out;
}
