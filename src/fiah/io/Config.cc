#include <filesystem>
#include <iostream>

#include <toml.hpp>

#include "fiah/utils/Timer.hpp"
#include "fiah/io/Config.hh"
#include "fiah/io/Config.hh"

namespace fiah::io {

Config::Config(const std::filesystem::path& path)
    : m_config_path{path}
{}

const std::string& Config::get_market_ip() const noexcept { return m_market_ip; }


uint16_t Config::get_market_port() noexcept { return m_market_port; }

bool Config::parse_config() noexcept
{
    utils::Timer timer{"Config::parse_config()"};
    const auto root = toml::parse(m_config_path);
    assert(root.at("title").as_string() == "HFT Config");

    const toml::value& market = root.at("servers").at("market");

    const auto& market_ip = market.at("ip").as_string();
    const auto& market_port = market.at("port").as_integer();

    m_market_port = market_port;
    m_market_ip = market_ip;
    return true;
}
} // End io namespace