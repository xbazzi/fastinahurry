#pragma once

#include "PayOff.hh"

namespace options {

class PayOffCall: public PayOff {
private:
    double _K;

public:
    PayOffCall(const double K);
    virtual ~PayOffCall() {};
    virtual double operator()(const double S) const; // max(S - K, 0)
};
} // End namespace options