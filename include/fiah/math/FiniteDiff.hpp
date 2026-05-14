#pragma once

#include <concepts>
#include <functional>
#include <utility>

namespace fiah
{
class FiniteDiff
{
  public:
    enum class DiffType
    {
        Forward,
        Backward,
        Central
    };

    template <class F>
        requires std::invocable<F, double>
    static double calc(double x, double h, F &&f, DiffType type = DiffType::Forward)
    {
        switch (type)
        {
        case DiffType::Forward:
            return calcForwardFiniteDiff(x, std::forward<F>(f), h);
        case DiffType::Central:
            return calcCentralFiniteDiff(x, std::forward<F>(f), h);
        case DiffType::Backward:
            return calcBackwardFiniteDiff(x, std::forward<F>(f), h);
        default:
            std::unreachable();
        }
    }

  private:
    template <class F> [[nodiscard]] static double calcForwardFiniteDiff(double x, F &&f, double h)

    {
        return (std::invoke(f, x + h) - std::invoke(f, x)) / h;
    }

    template <class F> [[nodiscard]] static double calcCentralFiniteDiff(double x, F &&f, double h)
    {
        return (std::invoke(f, x + h) - std::invoke(f, x - h)) / (2.0 * h);
    }

    template <class F> [[nodiscard]] static double calcBackwardFiniteDiff(double x, F &&f, double h)
    {
        return (std::invoke(f, x) - std::invoke(f, x - h)) / h;
    }
};

} // end namespace fiah
