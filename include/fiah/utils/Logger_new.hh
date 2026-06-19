// // clang-format off
// // C++ Includes
// #include <cstdint>
// #include <cstddef>
// #include <source_location>

// // Third Party Includes

// // FastInAHurry Includes
// #include "fiah/utils/Timer.hh"
// #include "fiah/structs/SPSCQueue.hh"

// namespace fiah
// {

// /// @todo use fmt::
// template <class Tag>
// class Logger
// {
// public:
//     using Location = std::source_location;

//     enum class Level : std::uint8_t
//     {
//         INFO,
//         DEBUG,
//         ERROR,
//         WARN
//     };

//     static constexpr std::uint16_t QUEUE_SIZE{4096U};
//     using queue_t = fiah::SPSCQueue<std::byte, QUEUE_SIZE>;

//     __always_inline
//     static Logger& get_instance()
//     {
//         static Logger instance{};
//         return instance;
//     }

//     template <class... Args>
//     __always_inline
//     void info(Location location, int line, Args&& ...args)
//     {
//         _log(Level::INFO, location.file_name(), location.function_name(), location.line(), std::forward<Args>(args)...);
//     }

//     template <class... Args>
//     __always_inline
//     void warn(Location location, int line, Args&& ...args)
//     {
//         _log(Level::WARN, location.file_name(), location.function_name(), location.line(), std::forward<Args>(args)...);
//     }

//     template <class... Args>
//     __always_inline
//     void error(Location location, int line, Args&& ...args)
//     {
//         _log(Level::ERROR, location.file_name(), location.function_name(), location.line(), std::forward<Args>(args)...);
//     }

// protected:
//     ~Logger();

// private:
//     Logger()                          = default;
//     Logger(const Logger &)            = delete;
//     Logger(Logger &&)                 = delete;
//     Logger &operator=(Logger &&)      = delete;
//     Logger &operator=(const Logger &) = delete;

//     template <class... Args> 
//     __always_inline
//     void _log(Logger::Level level, const char* /*file_name*/, const char* /*function_name*/, const char* /*line*/, 
//               std::uint32_t line, Args &&...args)
//     {
//         // LoggerImpl::log(/*implement*/);
//         return;
//     }

//     static const char *_level_to_string(Level level)
//     {
//         switch (level)
//         {
//         case Level::INFO:
//             return "INFO";
//         case Level::DEBUG:
//             return "DEBUG";
//         case Level::ERROR:
//             return "ERROR";
//         case Level::WARN:
//             return "WARN";
//         default:
//             std::unreachable();
//         }
//     }

//     static const char *_level_to_color(Level level)
//     {
//         switch (level)
//         {
//         case Level::INFO:
//             return "\033[32m"; // Green
//         case Level::DEBUG:
//             return "\033[36m"; // Cyan
//         case Level::ERROR:
//             return "\033[31m"; // Red
//         case Level::WARN:
//             return "\033[33m"; // Red
//         default:
//             return "\033[0m";
//         }
//     }

//     queue_t* m_pLogQ;
// };
// } // End namespace fiah