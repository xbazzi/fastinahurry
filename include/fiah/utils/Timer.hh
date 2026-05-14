// clang-format off
#pragma once

#include <chrono>
#include <cstdint>
#include <iostream>
#include <print>
#include <ratio>
#include <sstream>
#include <string_view>

namespace fiah
{
using namespace std::literals::chrono_literals;

template <class Clock = std::chrono::steady_clock>
    requires std::chrono::is_clock_v<Clock>
class Timer
{
  public:
    using clock_type    = Clock;
    using time_point    = typename clock_type::time_point;
    using duration_type = typename clock_type::duration;

    Timer() noexcept 
        :  m_startTimePoint{clock_type::now()}
    {
    }

    [[nodiscard]]
    __attribute__ ((__always_inline__))
    time_point startTime() const noexcept
    {
        return m_startTimePoint;
    }

    inline void reset() noexcept
    {
        m_startTimePoint = clock_type::now();
    }

    [[nodiscard]]
    duration_type elapsed() const noexcept
    {
        return clock_type::now() - m_startTimePoint;
    }

    template <class Duration = std::chrono::nanoseconds>
    [[nodiscard]]
    auto elapsedAs() const noexcept
    {
        return std::chrono::duration_cast<Duration>(elapsed());
    }

    [[nodiscard]]
    std::int64_t elapsedNs() const noexcept
    {
        return elapsedAs<std::chrono::nanoseconds>().count();
    }

    [[nodiscard]]
    std::int64_t elapsedUs() const noexcept
    {
        return elapsedAs<std::chrono::microseconds>().count();
    }

    [[nodiscard]]
    std::int64_t elapsedMs() const noexcept
    {
        return elapsedAs<std::chrono::milliseconds>().count();
    }

    [[nodiscard]]
    double elapsedSecs() const noexcept
    {
        return std::chrono::duration<double>(elapsed()).count();
    }

    Timer(const Timer &) = delete;
    Timer(Timer &&) = delete;
    Timer &operator=(const Timer &) = delete;
    Timer &operator=(Timer &&) = delete;

    ~Timer() noexcept
    {
        // stop();
    }

  private:
    time_point m_startTimePoint;
};
} // namespace fiah