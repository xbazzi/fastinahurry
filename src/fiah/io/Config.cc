#include <filesystem>
#include <iostream>

#include <toml.hpp>

#include "fiah/utils/Timer.hpp"
#include "fiah/io/Config.hh"

namespace fiah::io {

Config::Config(const std::filesystem::path& path)
    : m_config_path{path}
{}

bool Config::parse_config() noexcept
{
    utils::Timer timer{"Config::parse_config()"};
    const auto root = toml::parse(m_config_path);
    assert(root.at("title").as_string() == "HFT Config");

    // Parse logging configuration
    const toml::value& logging = root.at("logging");
    m_logging_enabled = logging.at("enabled").as_boolean();
    m_log_level = logging.at("level").as_string();
    m_log_to_file = logging.at("log_to_file").as_boolean();
    m_log_path = logging.at("log_path").as_string();
    m_max_file_size_mb = logging.at("max_file_size_mb").as_integer();
    m_rotation_count = logging.at("rotation_count").as_integer();

    // Parse trading configuration
    const toml::value& trading = root.at("trading");
    m_trading_enabled = trading.at("enabled").as_boolean();
    m_strategy = trading.at("strategy").as_string();
    m_max_position_size = trading.at("max_position_size").as_integer();
    m_max_order_size = trading.at("max_order_size").as_integer();
    m_risk_limit_usd = trading.at("risk_limit_usd").as_floating();
    m_tick_size = trading.at("tick_size").as_floating();
    m_latency_threshold_us = trading.at("latency_threshold_us").as_integer();

    // Parse network configuration
    const toml::value& network = root.at("network");
    m_tcp_buffer_size = network.at("tcp_buffer_size").as_integer();
    m_udp_buffer_size = network.at("udp_buffer_size").as_integer();
    m_socket_timeout_ms = network.at("socket_timeout_ms").as_integer();
    m_keepalive_interval_s = network.at("keepalive_interval_s").as_integer();
    m_max_connections = network.at("max_connections").as_integer();
    m_enable_nagle = network.at("enable_nagle").as_boolean();

    // Parse system configuration
    const toml::value& system = root.at("system");
    m_num_threads = system.at("num_threads").as_integer();

    const auto& cpu_affinity_array = system.at("cpu_affinity").as_array();
    m_cpu_affinity.clear();
    for (const auto& cpu : cpu_affinity_array) {
        m_cpu_affinity.push_back(cpu.as_integer());
    }

    m_use_huge_pages = system.at("use_huge_pages").as_boolean();
    m_priority = system.at("priority").as_string();
    m_watchdog_timeout_s = system.at("watchdog_timeout_s").as_integer();

    // Parse server configuration
    const toml::value& market = root.at("servers").at("market");
    m_market_ip = market.at("ip").as_string();
    m_market_port = market.at("port").as_integer();
    m_market_protocol = market.at("protocol").as_string();

    const toml::value& beta = root.at("servers").at("beta");
    m_beta_ip = beta.at("ip").as_string();
    m_beta_port = beta.at("port").as_integer();
    m_beta_protocol = beta.at("protocol").as_string();

    const toml::value& risk = root.at("servers").at("risk");
    m_risk_ip = risk.at("ip").as_string();
    m_risk_port = risk.at("port").as_integer();
    m_risk_protocol = risk.at("protocol").as_string();

    // Parse database configuration
    const toml::value& database = root.at("database");
    m_database_enabled = database.at("enabled").as_boolean();
    m_database_host = database.at("host").as_string();
    m_database_port = database.at("port").as_integer();
    m_database_name = database.at("name").as_string();
    m_database_user = database.at("user").as_string();
    m_database_password = database.at("password").as_string();
    m_database_connection_pool_size = database.at("connection_pool_size").as_integer();

    return true;
}

// Logging getters
bool Config::get_logging_enabled() const noexcept { return m_logging_enabled; }
const std::string& Config::get_log_level() const noexcept { return m_log_level; }
bool Config::get_log_to_file() const noexcept { return m_log_to_file; }
const std::string& Config::get_log_path() const noexcept { return m_log_path; }
int Config::get_max_file_size_mb() const noexcept { return m_max_file_size_mb; }
int Config::get_rotation_count() const noexcept { return m_rotation_count; }

// Trading getters
bool Config::get_trading_enabled() const noexcept { return m_trading_enabled; }
const std::string& Config::get_strategy() const noexcept { return m_strategy; }
int Config::get_max_position_size() const noexcept { return m_max_position_size; }
int Config::get_max_order_size() const noexcept { return m_max_order_size; }
double Config::get_risk_limit_usd() const noexcept { return m_risk_limit_usd; }
double Config::get_tick_size() const noexcept { return m_tick_size; }
int Config::get_latency_threshold_us() const noexcept { return m_latency_threshold_us; }

// Network getters
int Config::get_tcp_buffer_size() const noexcept { return m_tcp_buffer_size; }
int Config::get_udp_buffer_size() const noexcept { return m_udp_buffer_size; }
int Config::get_socket_timeout_ms() const noexcept { return m_socket_timeout_ms; }
int Config::get_keepalive_interval_s() const noexcept { return m_keepalive_interval_s; }
int Config::get_max_connections() const noexcept { return m_max_connections; }
bool Config::get_enable_nagle() const noexcept { return m_enable_nagle; }

// System getters
int Config::get_num_threads() const noexcept { return m_num_threads; }
const std::vector<int>& Config::get_cpu_affinity() const noexcept { return m_cpu_affinity; }
bool Config::get_use_huge_pages() const noexcept { return m_use_huge_pages; }
const std::string& Config::get_priority() const noexcept { return m_priority; }
int Config::get_watchdog_timeout_s() const noexcept { return m_watchdog_timeout_s; }

// Server getters
const std::string& Config::get_market_ip() const noexcept { return m_market_ip; }
uint16_t Config::get_market_port() const noexcept { return m_market_port; }
const std::string& Config::get_market_protocol() const noexcept { return m_market_protocol; }

const std::string& Config::get_beta_ip() const noexcept { return m_beta_ip; }
uint16_t Config::get_beta_port() const noexcept { return m_beta_port; }
const std::string& Config::get_beta_protocol() const noexcept { return m_beta_protocol; }

const std::string& Config::get_risk_ip() const noexcept { return m_risk_ip; }
uint16_t Config::get_risk_port() const noexcept { return m_risk_port; }
const std::string& Config::get_risk_protocol() const noexcept { return m_risk_protocol; }

// Database getters
bool Config::get_database_enabled() const noexcept { return m_database_enabled; }
const std::string& Config::get_database_host() const noexcept { return m_database_host; }
uint16_t Config::get_database_port() const noexcept { return m_database_port; }
const std::string& Config::get_database_name() const noexcept { return m_database_name; }
const std::string& Config::get_database_user() const noexcept { return m_database_user; }
const std::string& Config::get_database_password() const noexcept { return m_database_password; }
int Config::get_database_connection_pool_size() const noexcept { return m_database_connection_pool_size; }

} // End io namespace