#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <thread>

#include "io/TcpServer.hh"
#include "MarketData.hh"

namespace io {

TcpServer::TcpServer(const std::string& ip, uint16_t port)
    : _ip{ip}, _port{port} 
{
    static_assert(!std::is_copy_constructible_v<TcpServer>);
    static_assert(!std::is_copy_assignable<TcpServer>::value);
    static_assert(std::is_move_constructible_v<TcpServer>);
    static_assert(std::is_move_assignable_v<TcpServer>);
}

SocketRAII TcpServer::accept_client()
{
    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);

    SocketRAII client_fd{accept(_sock, reinterpret_cast<sockaddr*>(&client_addr), &client_len)};
    if (client_fd < 0) 
    {
        perror("accept");
        return SocketRAII{};
    }

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, std::addressof(client_addr.sin_addr), client_ip, sizeof(client_ip));
    std::cout << "[Server] Connection from " << client_ip << '\n';
    return client_fd;
}

ssize_t TcpServer::send(SocketRAII& client, const void* buf, size_t len)
{
    return ::send(client, buf, len, MSG_NOSIGNAL);
}

ssize_t TcpServer::recv(SocketRAII& client, void* buf, size_t len)
{
    return ::recv(client, buf, len, 0);
}

bool TcpServer::start() 
{
    _sock = socket(AF_INET, SOCK_STREAM, 0);

    if (_sock < 0) {
        perror("socket");
        return false;
    }

    int opt = 1;
    if (setsockopt(_sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        return false;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(_port);
    addr.sin_addr.s_addr = inet_addr(_ip.c_str());

    if (bind(_sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        perror("bind");
        return false;
    }

    if (listen(_sock, 3) < 0) {
        perror("listen");
        return false;
    }
    std::cout << "[Server] Listening on " << _ip << ":" << _port << std::endl;
    _running = true;
    return _running;
}
} // End namespace io