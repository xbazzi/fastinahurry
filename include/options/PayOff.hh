#pragma once

#include <algorithm>

namespace options {

class PayOff 
{
public:
    PayOff();
    virtual ~PayOff() {};

    virtual double operator()(const double S) const = 0;
};
} // End namespace options