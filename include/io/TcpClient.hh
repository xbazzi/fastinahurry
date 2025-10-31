#pragma once 

#include <cstdint>
#include "io/Tcp.hh"

namespace io {

class TcpClient : public Tcp
{
public:
    using Tcp::send_data;
    using Tcp::recv_data;

    TcpClient() = default;
    explicit TcpClient(const std::string&, const uint16_t);

    bool connect_to_server();
    ssize_t send(const void*, size_t);
    ssize_t recv(void*, size_t);
};
} // End namespace io