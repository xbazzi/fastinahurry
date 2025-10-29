#pragma once
#include <filesystem>
#include <cstdint>
#include <string>
#include <cstdint>

namespace io {
class Config {
private:
    std::string _market_ip;
    uint16_t _market_port;
    std::filesystem::path _config_path;
public:
    Config() noexcept = default;
    Config(const std::filesystem::path&) noexcept;
    ~Config() = default;

    bool parse_config() noexcept;
    std::string& get_market_ip() noexcept;
    uint16_t get_market_port() noexcept;
};
} // End io namespace