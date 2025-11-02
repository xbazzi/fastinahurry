#pragma once

#include <iostream>
#include <string>
#include <cstdint>
#include <sstream>
#include <type_traits>

namespace utils {
using namespace std::literals::string_literals;

class Logger
{
private:
    enum class Level : std::uint8_t
    {
        INFO,
        DEBUG,
        ERROR,
    };
    const std::string m_name;

    Logger(std::string name = "Generic Logger"s) 
        : m_name{std::move(name)} {}

    Logger(const Logger&)            = delete;
    Logger(Logger&&)                 = delete;
    Logger& operator=(Logger&&)      = delete;
    Logger& operator=(const Logger&) = delete;

    static const char* _level_to_string(Level lvl)
    {
        switch (lvl)
        {
            case Level::INFO:  return "INFO";
            case Level::DEBUG: return "DEBUG";
            case Level::ERROR: return "ERROR";
            default:           return "UNKNOWN";
        }
    }

    static const char* _level_to_color(Level lvl)
    {
        switch (lvl)
        {
            case Level::INFO:  return "\033[32m"; // Green
            case Level::DEBUG: return "\033[36m"; // Cyan
            case Level::ERROR: return "\033[31m"; // Red
            default:           return "\033[0m";
        }
    }

    /// @brief 
    /// @param level    Log level 
    /// @param ...args  Stuff to log (must implement <<operator)
    ///
    /// @todo Implement `has_stream_operator` concept
    template <class... Args>
    void _log(Logger::Level level, Args&&... args)
    {
        auto curr_time = std::chrono::utc_clock::now();
        // auto now = std::chrono::utc_clock::to_time_t(now);
        std::ostrinstream oss;
        oss << "[" << curr_time            << "]" 
            << "[" << m_name               << "]"
            << "[" << level_to_string(lvl) << "]"

        std::cout << level_to_color(lvl) << oss.str();
        (std::cout << ... << std::forward<Args>(args)) << "\033[0m" << std::endl;
    }

public:

    static Logger& get_instance(std::string name = "Generic Logger"s)
    {
        static Logger instance{std::move(name)};
        return instance;
    }

    template <class... Args>
    void info(Args&&... args) { _log(Level::INFO, std::forward<Args>(args)...); }

    template <class... Args>
    void error(Args&&... args) { _log(Level::ERROR, std::forward<Args>(args)...); }

    template <class... Args>
    void debug(Args&&... args) { _log(Level::DEBUG, std::forward<Args>(args)...); }

    const std::string& const get_name() const { return m_name; }

    void set_name(const std::string &name) { m_name = name; }

    /// @todo Research this. Could potentially break stuff
    // friend std::ostream &operator<<(std::ostream &os, const Logger &rhs) {
    //     os << "name: " << rhs.m_name;
    //     return os;
    // }

};
} // End namespace utils