#pragma once

#include <cstdint>
#include <chrono>
#include <type_traits>
#include <ratio>

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
    using Duration = ResolutionDuration<R>::type;
    using Rep = Duration::rep;
    using TimePoint = std::chrono::time_point<Clock, Duration>;

    constexpr TimeStamp() noexcept
    {
        update_now();
    }

    explicit TimeStamp(Rep ticks) noexcept : m_ticks{ticks} {}

    TimeStamp& update_now() noexcept
    {
        const auto now = Clock::now().time_since_epoch();
        m_ticks = std::chrono::duration_cast<Duration>(now).count();
        return *this;
    }

    Rep get_ticks() const noexcept
    {
        return m_ticks;
    }

    static float to_micro(const Rep val) noexcept
    {
        using ratio = std::ratio_divide<typename Duration::period, std::chrono::microseconds::period>;
        constexpr float factor = static_cast<float>(ratio::num) / static_cast<float>(ratio::den);
        return static_cast<float>(val) * factor;
    }

    template <class OtherDuration>
    constexpr void increment(OtherDuration dur) noexcept
    {
        m_ticks += std::chrono::duration_cast<Duration>(dur).count();
    }

    constexpr auto operator<=>(const TimeStamp&) const noexcept = default;

private:
    Rep m_ticks;
};
} // End namespace fiah