#pragma once

#include <limits>

#include "Order.hh"

struct Constants
{
    static const Price InvalidPrice = std::numeric_limits<Price>::quiet_NaN();
};