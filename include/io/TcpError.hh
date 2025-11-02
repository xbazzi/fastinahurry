#pragma once

#include <cstdint>

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