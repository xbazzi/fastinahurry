#pragma once

#include <cstdint>

namespace fiah
{
enum class TomlParserError : std::uint8_t
{
    FILE_NOT_FOUND,
    PARSE_ERROR,
    KEY_NOT_FOUND,
    TYPE_MISMATCH,
    INVALID_TOML
};

enum class Error : std::uint8_t
{
    USER_ERROR,
    CONFIG_ERROR,
    UNKNOWN_ERROR,
    INIT_ERROR,
    CONTROLLER_ERROR,
    CORE_ERORR,
    THREAD_ERROR
};

enum class TcpError : std::uint8_t
{
    BAD_SOCKET,
    BIND_FAIL,
    LISTEN_FAIL,
    CONNECT_FAIL,
    ACCEPT_FAIL,
    SEND_FAIL,
    RECV_FAIL,
    INVALID_IP
};

enum class UdpError : std::uint8_t
{
    BAD_SOCKET,
    BIND_FAIL,
    LISTEN_FAIL,
    CONNECT_FAIL,
    ACCEPT_FAIL,
    SEND_FAIL,
    RECV_FAIL,
    INVALID_IP
};
} // namespace fiah