#include <toml.hpp>
#include "io/Config.hh"
#include <filesystem>
#include <iostream>
#include "utils/Timer.hpp"

namespace io {
Config::Config(const std::filesystem::path& config_path) noexcept
 : _config_path{config_path}
{ }

std::string& Config::get_market_ip() noexcept { return _market_ip; }

uint16_t Config::get_market_port() noexcept { return _market_port; }

bool Config::parse_config() noexcept
{
    utils::Timer{"Config::parse_config()"};
    const auto root = toml::parse(_config_path);
    assert(root.at("title").as_string() == "HFT Config");

    const toml::value& market = root.at("servers").at("market");

    const auto& market_ip = market.at("ip").as_string();
    const auto& market_port = market.at("port").as_integer();

    _market_port = market_port;
    _market_ip = market_ip;
    return true;
}
} // End io namespace