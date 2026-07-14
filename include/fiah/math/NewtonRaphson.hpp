#pragma once

// C++ Includes
#include <cmath>
#include <concepts>
#include <cstdint>
#include <limits>

// FastInAHurry Includes
#include "FiniteDiff.hpp"

namespace fiah
{
class NewtonRaphson
{
  public:
    template <class F>
        requires std::invocable<F, double>
    [[nodiscard]]
    static double calc(double guess, F &&f, double delta = 0.05, double stop_tol = 1E-12)
    {
        return newton_raphson<F>(guess, std::forward<F>(f), delta, stop_tol);
    }

  private:
    template <class F>
        requires std::invocable<F, double>
    [[nodiscard]]
    static double newton_raphson(double guess, F &&f, double delta, double stop_tol, double max_iter = 1000)
    {
        double tol = std::numeric_limits<double>::max();
        uint32_t count{};
        double x = guess;

        while (tol > stop_tol && count < max_iter)
        {
            double new_x = x - f(x) / FiniteDiff::calc(x, delta, f);
            tol = std::abs(x - new_x);
            x = new_x;
            ++count;
        }
        return x;
    }
};
} // end namespace fiah