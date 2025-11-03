#pragma once

#include <filesystem>
#include <cstdint>
#include <string>
#include <cstdint>

namespace fiah::io {

class Config 
{
private:
    std::string m_market_ip;
    uint16_t m_market_port;
    std::filesystem::path m_config_path;
public:
    Config() noexcept = default;
    // Config(const std::filesystem::path&) noexcept;
    Config(const std::filesystem::path&);
    ~Config() = default;

    bool parse_config() noexcept;
    const std::string& get_market_ip() const noexcept;
    uint16_t get_market_port() noexcept;

};
} // End io namespace