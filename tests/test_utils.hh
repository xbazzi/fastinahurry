#pragma once

#include <gtest/gtest.h>
#include <filesystem>
#include <memory>
#include <string>
#include <fstream>

#include "fiah/io/Config.hh"

namespace fiah::testing {

/// @brief Utility class for managing test configurations
class TestConfigBuilder
{
private:
    std::string m_market_ip{"127.0.0.1"};
    std::uint16_t m_market_port{1337};
    std::filesystem::path m_config_path;
    bool m_use_temp_file{false};

public:
    TestConfigBuilder() = default;

    /// @brief Set custom market IP
    TestConfigBuilder& with_market_ip(const std::string& ip)
    {
        m_market_ip = ip;
        return *this;
    }

    /// @brief Set custom market port
    TestConfigBuilder& with_market_port(std::uint16_t port)
    {
        m_market_port = port;
        return *this;
    }

    /// @brief Use existing config file
    TestConfigBuilder& with_config_path(const std::filesystem::path& path)
    {
        m_config_path = path;
        m_use_temp_file = false;
        return *this;
    }

    /// @brief Create a temporary config file for testing
    TestConfigBuilder& with_temp_config()
    {
        m_use_temp_file = true;
        return *this;
    }

    /// @brief Build and return the config object
    io::Config build()
    {
        if (m_use_temp_file)
        {
            m_config_path = create_temp_config_file();
        }
        else if (m_config_path.empty())
        {
            m_config_path = "../config.toml";
        }

        io::Config config(m_config_path);
        config.parse_config();
        return config;
    }

private:
    std::filesystem::path create_temp_config_file()
    {
        auto temp_path = std::filesystem::temp_directory_path() / "fiah_test_config.toml";

        std::ofstream file(temp_path);
        file << "title = \"HFT Config\"\n\n";
        file << "[servers]\n\n";
        file << "[servers.market]\n";
        file << "ip = \"" << m_market_ip << "\"\n";
        file << "port = " << m_market_port << "\n";
        file << "role = \"publisher\"\n";
        file.close();

        return temp_path;
    }
};

/// @brief Helper to create default test config
inline io::Config create_default_test_config()
{
    return TestConfigBuilder()
        .with_config_path("../config.toml")
        .build();
}

/// @brief Helper to create config with custom port
inline io::Config create_test_config_with_port(std::uint16_t port)
{
    return TestConfigBuilder()
        .with_market_port(port)
        .with_temp_config()
        .build();
}

/// @brief RAII wrapper for cleaning up temp files
class TempFileGuard
{
private:
    std::filesystem::path m_path;
    bool m_should_delete;

public:
    explicit TempFileGuard(std::filesystem::path path, bool should_delete = true)
        : m_path(std::move(path)), m_should_delete(should_delete)
    {}

    ~TempFileGuard()
    {
        if (m_should_delete && std::filesystem::exists(m_path))
        {
            std::filesystem::remove(m_path);
        }
    }

    TempFileGuard(const TempFileGuard&) = delete;
    TempFileGuard& operator=(const TempFileGuard&) = delete;
    TempFileGuard(TempFileGuard&&) = default;
    TempFileGuard& operator=(TempFileGuard&&) = default;

    const std::filesystem::path& path() const { return m_path; }
};

/// @brief Test helper to capture log output (placeholder for future implementation)
class LogCapture
{
public:
    LogCapture() = default;

    // Future: Capture log output for verification
    std::string get_logs() const { return ""; }
    void clear() {}
};

} // namespace fiah::testing
