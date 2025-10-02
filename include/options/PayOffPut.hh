#pragma once

#include "PayOff.hh"

namespace options {

class PayOffPut: public PayOff {
private:
    double _K;

public:
    PayOffPut(const double K);
    virtual ~PayOffPut() {};
    virtual double operator()(const double S) const; // max(S - K, 0)
};
} // End namespace options