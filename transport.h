#ifndef TRANSPORT_H
#define TRANSPORT_H


class transport
{
private:
    float in,out;
public:
    transport();
    transport(float h);
    float output(float weight);
};

#endif // TRANSPORT_H
