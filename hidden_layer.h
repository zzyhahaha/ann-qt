#ifndef HIDDEN_LAYER_H
#define HIDDEN_LAYER_H
#include"neure.h"

class hidden_layer
{
private:
    float in,out;
    int key;
    int posx,posy;
public:
    hidden_layer();
    hidden_layer(int n);
    float Input();
    float Output();
    void Activation_Function(int flag);         //激活函数
};

#endif // HIDDEN_LAYER_H
