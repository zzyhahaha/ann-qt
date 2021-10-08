#ifndef HIDDEN_LAYER_H
#define HIDDEN_LAYER_H
#include"neure.h"

class hidden_layer
{
private:
    float in,out;
    int key;
public:
    hidden_layer();
    hidden_layer(int n);
    float Input();
    float Output();
};

#endif // HIDDEN_LAYER_H
