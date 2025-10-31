#include <arpa/inet.h>

#include "market/MarketData.hh"
#include "io/TcpClient.hh"

namespace io {

TcpClient::TcpClient(const std::string& ip, const uint16_t port)
    : Tcp{ip, port} {}

bool TcpClient::connect_to_server()
{
    if (!create_socket())
        return false;
    
    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = ::htons(_port);
    if (::inet_pton(AF_INET, _ip.c_str(), &server.sin_addr) <= 0)
        return false;

    if (::connect(_sock, reinterpret_cast<sockaddr*>(&server), sizeof(server)) <= 0)
        return false;
    return true;
}

[[gnu::hot]] ssize_t TcpClient::send(const void* buf, size_t len)
{
    return send_data(buf, len);
}

[[gnu::hot]] ssize_t TcpClient::recv(void* buf, size_t len)
{
    return recv_data(buf, len);
}
} // End namespace io