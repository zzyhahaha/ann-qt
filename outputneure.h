#ifndef OUTPUTNEURE_H
#define OUTPUTNEURE_H
#include "neure.h"

 class OutputNeure
{
 private:
     float in,out;
     int key;
public:
     OutputNeure();
     OutputNeure(int n);
    float input();

    float output();
};

#endif // OUTPUTNEURE_H
