#pragma once

#include <string>
#include <optional>
#include <cstdint>
#include <atomic>
#include <sys/types.h>
#include "io/SocketRAII.hpp"

namespace io {
class TcpServer {
private:
    std::string _ip{};
    uint16_t _port{};
    SocketRAII _sock{-1};

public:
    TcpServer() = default;
    explicit TcpServer(const std::string&, uint16_t);

    bool start();
    ssize_t send(SocketRAII&, const void*, size_t);
    ssize_t recv(SocketRAII&, void*, size_t);
    SocketRAII accept_client();
    bool _running{false};
};
} // End namespace io