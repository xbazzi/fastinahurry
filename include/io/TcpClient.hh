#pragma once 

#include <string>
#include <cstdint>
#include <optional>
#include "io/SocketRAII.hpp"

namespace io {

class TCPSubscriber
{
private:
    std::string _ip;
    uint16_t _port;
    SocketRAII _sock;
public:
    TCPSubscriber();
    TCPSubscriber(const std::string& ip, const uint16_t _port);
    std::optional<bool> init_socket();
};
} // End namespace io