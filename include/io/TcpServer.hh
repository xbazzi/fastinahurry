#pragma once

#include <cstdint>
#include "io/Tcp.hh"

namespace io {

class TcpServer : public Tcp
{
public:
    TcpServer() = default;
    explicit TcpServer(const std::string&, std::uint16_t);

    bool start();
    SocketRAII accept_client();
    ssize_t send(SocketRAII&, const void*, size_t);
    ssize_t recv(SocketRAII&, void*, size_t);
};
} // End namespace io