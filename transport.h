#ifndef TRANSPORT_H
#define TRANSPORT_H
#include"inputneure.h"
#include"outputnerue.h"
#include"hidden_layer.h"
#include"neure.h"

class transport
{
 protected:
    float in,out;
    float weight;
    int from,to;
    int key;//判断第几根线
    int posx,posy;
public:
    transport();
    transport(float h);
    void setto(int key);
    void setfrom(int key);
    void setWeight(float weight);
    int getfrom();
    int getto();
    int getkey();
    float getWeight();
    float input(float in);
    float output(float weight);
    void Delete();
};

#endif // TRANSPORT_H
