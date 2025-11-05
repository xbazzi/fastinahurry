#pragma once

#include <filesystem>
#include <cstdint>
#include <string>
#include <vector>

namespace fiah::io {

class Config
{
private:
    std::filesystem::path m_config_path;

    // Logging configuration
    bool m_logging_enabled;
    std::string m_log_level;
    bool m_log_to_file;
    std::string m_log_path;
    int m_max_file_size_mb;
    int m_rotation_count;

    // Trading configuration
    bool m_trading_enabled;
    std::string m_strategy;
    int m_max_position_size;
    int m_max_order_size;
    double m_risk_limit_usd;
    double m_tick_size;
    int m_latency_threshold_us;

    // Network configuration
    int m_tcp_buffer_size;
    int m_udp_buffer_size;
    int m_socket_timeout_ms;
    int m_keepalive_interval_s;
    int m_max_connections;
    bool m_enable_nagle;

    // System configuration
    int m_num_threads;
    std::vector<int> m_cpu_affinity;
    bool m_use_huge_pages;
    std::string m_priority;
    int m_watchdog_timeout_s;

    // Server configuration
    std::string m_market_ip;
    uint16_t m_market_port;
    std::string m_market_protocol;

    std::string m_beta_ip;
    uint16_t m_beta_port;
    std::string m_beta_protocol;

    std::string m_risk_ip;
    uint16_t m_risk_port;
    std::string m_risk_protocol;

    // Database configuration
    bool m_database_enabled;
    std::string m_database_host;
    uint16_t m_database_port;
    std::string m_database_name;
    std::string m_database_user;
    std::string m_database_password;
    int m_database_connection_pool_size;

public:
    Config() noexcept = default;
    Config(const std::filesystem::path&);
    ~Config() = default;

    bool parse_config() noexcept;

    // Logging getters
    bool get_logging_enabled() const noexcept;
    const std::string& get_log_level() const noexcept;
    bool get_log_to_file() const noexcept;
    const std::string& get_log_path() const noexcept;
    int get_max_file_size_mb() const noexcept;
    int get_rotation_count() const noexcept;

    // Trading getters
    bool get_trading_enabled() const noexcept;
    const std::string& get_strategy() const noexcept;
    int get_max_position_size() const noexcept;
    int get_max_order_size() const noexcept;
    double get_risk_limit_usd() const noexcept;
    double get_tick_size() const noexcept;
    int get_latency_threshold_us() const noexcept;

    // Network getters
    int get_tcp_buffer_size() const noexcept;
    int get_udp_buffer_size() const noexcept;
    int get_socket_timeout_ms() const noexcept;
    int get_keepalive_interval_s() const noexcept;
    int get_max_connections() const noexcept;
    bool get_enable_nagle() const noexcept;

    // System getters
    int get_num_threads() const noexcept;
    const std::vector<int>& get_cpu_affinity() const noexcept;
    bool get_use_huge_pages() const noexcept;
    const std::string& get_priority() const noexcept;
    int get_watchdog_timeout_s() const noexcept;

    // Server getters
    const std::string& get_market_ip() const noexcept;
    uint16_t get_market_port() const noexcept;
    const std::string& get_market_protocol() const noexcept;

    const std::string& get_beta_ip() const noexcept;
    uint16_t get_beta_port() const noexcept;
    const std::string& get_beta_protocol() const noexcept;

    const std::string& get_risk_ip() const noexcept;
    uint16_t get_risk_port() const noexcept;
    const std::string& get_risk_protocol() const noexcept;

    // Database getters
    bool get_database_enabled() const noexcept;
    const std::string& get_database_host() const noexcept;
    uint16_t get_database_port() const noexcept;
    const std::string& get_database_name() const noexcept;
    const std::string& get_database_user() const noexcept;
    const std::string& get_database_password() const noexcept;
    int get_database_connection_pool_size() const noexcept;
};
} // End io namespace
