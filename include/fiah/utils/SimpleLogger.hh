#pragma once

#include "fiah/utils/Logger.hh"
#include <cstdint>
#include <source_location>
#include <string>
#include <utility>

namespace fiah::log
{

// Internal tag type used to instantiate the underlying templated Logger.
struct _InternalLoggerTag
{
};

// Access the underlying Logger singleton for the facade
inline fiah::utils::Logger<_InternalLoggerTag> &impl_logger()
{
    // Return the singleton instance. If you want to control the logger name,
    // call `fiah::log::init(name)` before any other logging occurs so the
    // singleton is constructed with your desired name.
    return fiah::utils::Logger<_InternalLoggerTag>::get_instance();
}

// Initialize/set the backend logger name
inline void init(std::string name)
{
    // Ensure the Meyers singleton is constructed with the provided name.
    // This must be called before any other call that would construct the
    // singleton (i.e., before the first log call) to take effect.
    (void)fiah::utils::Logger<_InternalLoggerTag>::get_instance(std::move(name));
}

// Thin forwarding APIs. These mirror the Logger methods but avoid the need
// for template parameters and instance variables.
template <class... Args>
inline void info(const std::source_location &loc = std::source_location::current(), Args &&...args)
{
    impl_logger().info(loc.function_name(), static_cast<std::uint32_t>(loc.line()), std::forward<Args>(args)...);
}

template <class... Args>
inline void debug(const std::source_location &loc = std::source_location::current(), Args &&...args)
{
    impl_logger().debug(loc.function_name(), static_cast<int>(loc.line()), std::forward<Args>(args)...);
}

template <class... Args>
inline void error(const std::source_location &loc = std::source_location::current(), Args &&...args)
{
    impl_logger().error(loc.function_name(), static_cast<int>(loc.line()), std::forward<Args>(args)...);
}

template <class... Args>
inline void warn(const std::source_location &loc = std::source_location::current(), Args &&...args)
{
    impl_logger().warn(loc.function_name(), static_cast<int>(loc.line()), std::forward<Args>(args)...);
}

// Note: these macros use std::source_location to capture caller info.
#define LOG_INFO_S(...) ::fiah::log::info(std::source_location::current(), __VA_ARGS__)
#define LOG_DEBUG_S(...) ::fiah::log::debug(std::source_location::current(), __VA_ARGS__)
#define LOG_ERROR_S(...) ::fiah::log::error(std::source_location::current(), __VA_ARGS__)
#define LOG_WARN_S(...) ::fiah::log::warn(std::source_location::current(), __VA_ARGS__)

} // namespace fiah::log
