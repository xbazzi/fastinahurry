#pragma once

#include <cstdint>
#include <chrono>

namespace fiah 
{
enum class Resolution : std::uint8_t
{
    Seconds,
    Milli,
    Micro,
    Nano
};

template <Resolution R>
struct ResolutionDuration;

template <>
struct ResolutionDuration<Resolution::Seconds>
{
    using type = std::chrono::seconds;
};

template <>
struct ResolutionDuration<Resolution::Milli>
{
    using type = std::chrono::milliseconds;
};

template <>
struct ResolutionDuration<Resolution::Micro>
{
    using type = std::chrono::microseconds;
};

template <>
struct ResolutionDuration<Resolution::Nano>
{
    using type = std::chrono::nanoseconds;
};

template <Resolution R = Resolution::Nano, class Clock = std::chrono::steady_clock>
class TimeStamp
{
public:
    using clock = Clock;
    using duration = ResolutionDuration<R>::type;
    using rep = duration::rep;
    using time_point = std::chrono::time_point<clock, duration>;

    constexpr TimeStamp() noexcept = default;
    explicit TimeStamp(rep ticks) noexcept : m_ticks{ticks} {}

    void update_now() noexcept
    {
        const auto now = clock::now().time_since_epoch();
        m_ticks = std::chrono::duration_cast<duration>(now).count();
    }

    rep get_ticks() const noexcept
    {
        return m_ticks;
    }

    template <class Duration>
    constexpr void increment(Duration dur) noexcept
    {
        m_ticks += std::chrono::duration_cast<duration>(dur).count();
    }

    constexpr auto operator<=>(const TimeStamp&) const noexcept = default;

private:
    rep m_ticks;
};
} // End namespace fiah