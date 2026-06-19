#pragma once

#include <concepts>
#include <functional>
#include <utility>

namespace fiah
{
class FiniteDiff
{
  public:
    enum class DiffType : std::uint8_t
    {
        None = 0,
        Forward,
        Backward,
        Central
    };

    template <class FuncT, DiffType eDiffType>
        requires std::invocable<FuncT, double>
    static double calc(double x, double h, FuncT &&f)
    {
        if constexpr (eDiffType == DiffType::Forward)
            return calcForwardFiniteDiff(x, std::forward<F>(f), h);
        else if constexpr (eDiffType == DiffType::Central)
            return calcCentralFiniteDiff(x, std::forward<F>(f), h);
        else (eDiffType == DiffType::Backward)
            return calcBackwardFiniteDiff(x, std::forward<F>(f), h);
        std::unreachable();
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
