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
    float weight=0;
    int from,to;
    int key;//判断第几根线
    int posx1,posy1;
    int posx2,posy2;
public:
    transport();
    transport(float h);
    void setto(int key);
    void setfrom(int key);
    void setWeight(float weight);
    void setkey(int key);
    int getfrom();
    int getto();
    int getkey();
    int getposx1();
    int getposy1();
    int getposx2();
    int getposy2();
    float getWeight();
    float input(float in);
    float output(float weight);
    void Delete();
    void SetposFrom(int posx1,int posy1);
    void SetposTo(int posx2,int posy2);
};

#endif // TRANSPORT_H
