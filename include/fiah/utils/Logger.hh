#pragma once

#include <iostream>
#include <string>
#include <cstdint>
#include <sstream>
#include <type_traits>
#include <chrono>
#include <source_location>


using namespace std::literals::string_literals;
namespace fiah::utils {

template <class Tag>
class Logger
{

public:
    enum class Level : std::uint8_t
    {
        INFO,
        DEBUG,
        ERROR,
    };

    using Location = std::source_location;

    /// @brief Singleton-returning method
    /// @param location 
    /// @return Singleton per `Tag`, which is the class that uses this Logger
    static Logger& get_instance(Location location = Location::current())
    {
        static Logger instance{typeid(Tag).name()};
        return instance;
    }

    template <class... Args>
    void info(const char *func, int line, Args &&...args)
    {
        _log(Level::INFO, func, line, std::forward<Args>(args)...);
    }

    template <class... Args>
    void error(const char* func, int line, Args&&... args) 
    { 
        _log(Level::ERROR, func, line, std::forward<Args>(args)...); 
    }

    template <class... Args>
    void debug(const char* func, int line, Args&&... args) 
    { 
        _log(Level::DEBUG, func, line, std::forward<Args>(args)...); 
    }

    #define LOG_INFO(...) m_logger.info(__func__, __LINE__, __VA_ARGS__)
    #define LOG_DEBUG(...) m_logger.debug(__func__, __LINE__, __VA_ARGS__)
    #define LOG_ERROR(...) m_logger.error(__func__, __LINE__, __VA_ARGS__)

    /// @todo Research this. Could potentially break stuff
    // friend std::ostream &operator<<(std::ostream &os, const Logger &rhs) {
    //     os << "name: " << rhs.m_scope;
    //     return os;
    // }

private:

    std::string m_name;

    Logger()
        : m_name{"GenericLogger"} 
    {}

    Logger(std::string&& name) 
        : m_name{std::move(name)} 
    {}

    Logger(const Logger&)            = delete;
    Logger(Logger&&)                 = delete;
    Logger& operator=(Logger&&)      = delete;
    Logger& operator=(const Logger&) = delete;


    /// @brief 
    /// @param level    Log level 
    /// @param ...args  Stuff to log (must implement <<operator)
    ///
    /// @todo Implement `has_stream_operator` concept to assert << availability
    template <class... Args>
    void _log(Logger::Level level, const char* func, int line, Args&&... args)
    {
        auto now = std::chrono::system_clock::now();
        auto curr_time = std::chrono::system_clock::to_time_t(now);
        auto formatted_time = std::put_time(
            std::localtime(std::addressof(curr_time)),
            "%H:%M:%S.00"
        );
        // auto now = std::chrono::utc_clock::to_time_t(now);
        std::ostringstream oss;

        oss << '[' << formatted_time                            << ']' 
            << '[' << m_name << "::" << func << ':' << line     << ']'
            << '[' << _level_to_string(level)                   << "] ";

        std::cout << _level_to_color(level)
                  << oss.str();
        
        (std::cout << ... << std::forward<Args>(args)) 
            << "\033[0m" << std::endl;
    }

    static const char* _level_to_string(Level level)
    {
        switch (level)
        {
            case Level::INFO:  return "INFO";
            case Level::DEBUG: return "DEBUG";
            case Level::ERROR: return "ERROR";
            default:           return "UNKNOWN";
        }
    }

    static const char* _level_to_color(Level level)
    {
        switch (level)
        {
            case Level::INFO:  return "\033[32m"; // Green
            case Level::DEBUG: return "\033[36m"; // Cyan
            case Level::ERROR: return "\033[31m"; // Red
            default:           return "\033[0m";
        }
    }


};
} // End namespace fiah::utils