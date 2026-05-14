#include <functional>
#include <iostream>

#include "FiniteDiff.hpp"
#include "NewtonRaphson.hpp"

double f2(double x)
{
    return 2 * x * x - 4;
}

auto main() -> int
{
    using namespace fiah;
    // auto f = [](double d) -> double { return 2 * d * d; };
    std::cout << FiniteDiff::calc(3.0, 0.001, f2, FiniteDiff::DiffType::Forward) << '\n'
              << NewtonRaphson::calc(-3.0, f2, 0.0001);

    return 0;
}