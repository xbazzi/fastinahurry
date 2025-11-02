#pragma once

#include <chrono>
#include <cstdint>
#include <print>
#include <string_view>
#include <ratio>
#include <iostream>


namespace utils {
using namespace std::literals::chrono_literals;

class Timer
{
private:
    using clock = std::chrono::steady_clock;
    using time_point = std::chrono::time_point<clock>;
    time_point m_start_timepoint;
    std::string_view m_scope_name;


    [[nodiscard]] auto elapsed() const
    {
        return std::chrono::duration_cast<std::chrono::microseconds>
            (clock::now() - m_start_timepoint);
    }

    void stop()
    {
        std::chrono::microseconds dur = elapsed();
        // operator<<(((std::cout, dur), std::cout, " took a minute pham"));
        std::cout << "[Timer] " << m_scope_name
                << " took ≈ "
                << dur / 1us << "us\n";
        // std::print("{}[Timer] {} took {}ns", 
        //     std::chrono::utc_clock::now(),
        //     m_scope_name,
        //     dur.count());
    }
    void reset() { m_start_timepoint = clock::now(); }
public:
    Timer() noexcept 
        : m_scope_name{"Unspecified"}, m_start_timepoint{clock::now()}
    {}
    Timer(const std::string_view scope_name) noexcept
        : m_scope_name{scope_name}, m_start_timepoint{clock::now()}
    {}
    Timer(const Timer&)            = delete;
    Timer(Timer&&)                 = delete;
    Timer& operator=(const Timer&) = delete;
    Timer& operator=(Timer&&)      = delete;

    ~Timer() noexcept { stop(); }
};
} // End namespace utils 