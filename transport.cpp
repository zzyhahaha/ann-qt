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
