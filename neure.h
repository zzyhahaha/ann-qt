#ifndef NEURE_H
#define NEURE_H
#include<string>
using namespace std;

class Neure
{
private:
    int key;
    string kind;

public:
    Neure();
    Neure(int n);
    int getKey();
};

#endif // NEURE_H
