#pragma once

#include <algorithm>

class PayOff 
{
public:
    PayOff();
    virtual ~PayOff() {};

    virtual double operator()(const double S) const = 0;
};