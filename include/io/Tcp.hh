#pragma once

#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>

#include <string>
#include <cstdint>

#include "io/SocketRAII.hpp"
#include "io/TcpError.hh"

namespace io {

class Tcp
{
protected:
    std::string _ip{};
    std::uint16_t _port{};
    SocketRAII _sock{-1};
    bool _running{false};

    Tcp() = default;
    explicit Tcp(const std::string& ip, std::uint16_t port)
        : _ip{ip}, _port{port} 
    {
        static_assert(!std::is_copy_constructible_v<Tcp>);
        static_assert(!std::is_copy_assignable<Tcp>::value);
        static_assert(std::is_move_constructible_v<Tcp>);
        static_assert(std::is_move_assignable_v<Tcp>);
    }

    auto create_socket()
        -> std::expected<void, TcpError>
    {
        int fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0)
            return std::unexpected(TcpError::BAD_SOCKET);
        _sock = SocketRAII{fd};
        return {};
    }

    [[gnu::hot]] ssize_t send_data(const void* buf, size_t len, int flags = 0) 
    {
        return ::send(_sock, buf, len, flags);
    }

    [[gnu::hot]] ssize_t recv_data(void* buf, size_t len, int flags = 0) 
    {
        return ::recv(_sock, buf, len, flags);
    }

public:
    Tcp(const Tcp&)             = delete;
    Tcp& operator=(const Tcp&)  = delete;

    Tcp(Tcp&&)                  = default;
    Tcp& operator=(Tcp&&)       = default;

    virtual ~Tcp() noexcept     = default;
};
} // End namespace io