#pragma once

#include <cstdint>

namespace fiah {
enum class AlgoError : std::uint8_t
{
    INIT_CLIENT_FAIL,
    SERVER_NOT_ONLINE,
    INIT_SERVER_FAIL,
    INVALID_STATE
};

#pragma pack(push, 1)
enum class Error : std::uint8_t
{
    USER_ERROR,
    CONFIG_ERROR,
    UNKNOWN_ERROR,
    INIT_ERROR,
    CONTROLLER_ERROR,
    ALGO_ERORR,
    THREAD_ERROR
};
#pragma pack(pop)

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
} // End namespace fiah