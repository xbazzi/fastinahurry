#pragma once

#include <arpa/inet.h>

#include <cstdint>
#include <expected>

#include "fiah/io/Tcp.hh"

namespace fiah
{

class TcpClient : public Tcp
{
  public:
    using Tcp::recv_data;
    using Tcp::send_data;

    TcpClient() = default;
    explicit TcpClient(const std::string &ip, const std::uint16_t port) : Tcp{ip, port}
    {
    }
    explicit TcpClient(std::string &&ip, const std::uint16_t port) : Tcp{std::move(ip), port}
    {
    }

    inline std::expected<void, fiah::TcpError> connect_to_server()
    {
        if (!create_socket())
            return std::unexpected(fiah::TcpError::BAD_SOCKET);

        sockaddr_in server{};
        server.sin_family = AF_INET;
        server.sin_port = ::htons(_port);
        if (::inet_pton(AF_INET, _ip.c_str(), &server.sin_addr) <= 0)
            return std::unexpected(fiah::TcpError::INVALID_IP);

        if (::connect(m_sock, reinterpret_cast<sockaddr *>(&server), sizeof(server)) < 0)
            return std::unexpected(fiah::TcpError::CONNECT_FAIL);
        return {};
    }

    [[gnu::hot]] inline std::expected<std::uint64_t, fiah::TcpError> send(const void *buf, size_t len)
    {
        ssize_t result = Tcp::send_data(buf, len);
        if (result < 0) [[unlikely]]
            return std::unexpected(fiah::TcpError::SEND_FAIL);
        return static_cast<std::uint64_t>(result);
    }

    [[gnu::hot]] inline std::expected<std::uint64_t, fiah::TcpError> recv(void *buf, size_t len)
    {
        ssize_t result = Tcp::recv_data(buf, len);
        if (result < 0) [[unlikely]]
            return std::unexpected(fiah::TcpError::RECV_FAIL);
        return static_cast<std::uint64_t>(result);
    }
};

} // End namespace fiah