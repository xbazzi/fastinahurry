#include <toml.hpp>
#include "io/Config.hh"
#include <filesystem>
#include <iostream>

namespace io {
Config::Config(std::filesystem::path& config_path)
 : _config_path{config_path}
{ }

bool Config::parse_config() noexcept
{
    const auto root = toml::parse(_config_path);
    // assert(root.is_table());
    assert(root.at("title").as_string() == "TOML Example");

    std::cout << _config_path << std::endl;
    // std::cout << "COmments size: " << root.comments().size() << std::endl;
    // const toml::value& keys = root.at("key");
    const auto dob = root.at("owner").at("dob" ).as_offset_datetime();


    std::cout << root.at("owner").at("name").as_string() << std::endl;
    return 1;
}
} // End io namespace