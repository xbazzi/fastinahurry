#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <thread>
#include <exception>
#include <expected>

#include "io/TcpServer.hh"
#include "io/TcpError.hh"
#include "market/MarketData.hh"
#include "utils/Timer.hpp"
#include "utils/Logger.hh"

namespace io {

TcpServer::TcpServer(const std::string& ip, uint16_t port)
    : Tcp{ip, port}, m_logger{utils::Logger::get_instance("TcpServer")} {}

auto TcpServer::start()     
    -> std::expected<void, TcpError>
{
    utils::Timer timer{"TcpServer::start()"};
    m_logger.info("Attempting to start server on ", _ip, ":", _port);

    if (!create_socket())
    {
        m_logger.error("Couldn't create socket.");
        // throw std::runtime_error("Failed to create server socket: " + std::string(strerror(errno)));
        return std::unexpected(TcpError::BAD_SOCKET);
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(_port);
    addr.sin_addr.s_addr = inet_addr(_ip.c_str());

    if (::bind(_sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) 
    {
        m_logger.error("Couldn't bind address to socket.");
        // throw std::runtime_error("Failed to create server socket: " + std::string(strerror(errno)));
        return std::unexpected(TcpError::BIND_FAIL);
    }

    if (::listen(_sock, MAX_LISTEN_NUM) < 0) 
    {
        m_logger.error("Couldn't listen on socket.");
        return std::unexpected(TcpError::LISTEN_FAIL);
    }
    _running = true;
    m_logger.info("Server listening!");
    return {};
}

auto TcpServer::accept_client() 
    -> std::expected<SocketRAII, TcpError>
{
    int client_fd = ::accept(_sock, nullptr, nullptr);
    if (client_fd < 0)
        return std::unexpected(TcpError::BAD_SOCKET);
    return SocketRAII{client_fd};
}

[[gnu::hot]] auto TcpServer::send(SocketRAII& client, const void* buf, size_t len)
    -> std::expected<std::uint64_t, TcpError>
{
    utils::Timer timer{"TcpServer::send()"};
    ssize_t result = ::send(client, buf, len, MSG_NOSIGNAL);
    if (result < 0) [[unlikely]]
        return std::unexpected(TcpError::SEND_FAIL);
    return static_cast<std::uint64_t>(result);
}

[[gnu::hot]] auto TcpServer::recv(SocketRAII& client, void* buf, size_t len)
    -> std::expected<std::uint64_t, TcpError>
{
    utils::Timer timer{"TcpServer::recv()"};
    ssize_t result = ::recv(client, buf, len, 0);
    if (result < 0) [[unlikely]]
        return std::unexpected(TcpError::RECV_FAIL);
    return static_cast<std::uint64_t>(result);
}
} // End namespace io