#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <thread>

#include "io/TcpServer.hh"
#include "market/MarketData.hh"
#include "utils/Timer.hpp"

namespace io {

TcpServer::TcpServer(const std::string& ip, uint16_t port)
    : Tcp{ip, port} {}

bool TcpServer::start() 
{
    utils::Timer{"TcpServer::start()"};
    if (!create_socket())
        return false;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(_port);
    addr.sin_addr.s_addr = inet_addr(_ip.c_str());

    if (::bind(_sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        return false;
    }

    if (::listen(_sock, 5) < 0) 
    {
        return false;
    }
    _running = true;
    return true;
}

SocketRAII TcpServer::accept_client()
{
    int client_fd = ::accept(_sock, nullptr, nullptr);
    return SocketRAII{client_fd};
}

ssize_t TcpServer::send(SocketRAII& client, const void* buf, size_t len)
{
    utils::Timer{"TcpServer::send()"};
    return ::send(client, buf, len, MSG_NOSIGNAL);
}

ssize_t TcpServer::recv(SocketRAII& client, void* buf, size_t len)
{
    utils::Timer{"TcpServer::recv()"};
    return ::recv(client, buf, len, 0);
}

} // End namespace io