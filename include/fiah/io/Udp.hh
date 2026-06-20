#pragma once

// C++ Includes
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <cstdint>
#include <expected>
#include <string>

// FastInAHurry Includes
#include "fiah/error/Error.hh"
#include "fiah/io/Socket.hh"

namespace fiah {

class UdpBase
{
public:
    UdpBase(const UdpBase&) = delete;
    UdpBase& operator=(const UdpBase&) = delete;
    UdpBase(UdpBase&&) = default;
    UdpBase& operator=(UdpBase&&) = default;
    virtual ~UdpBase() noexcept = default;

protected:
    Socket m_sock;

protected:
    UdpBase() = default;

    auto create_socket() -> std::expected<void, UdpError>
    {
        int fd = ::socket(AF_INET, SOCK_DGRAM, 0);
        if (fd < 0)
            return std::unexpected(UdpError::BAD_SOCKET);
        m_sock = fd;
        return {};
    }

    int get_fd() const noexcept { return static_cast<int>(m_sock); }

    [[nodiscard, gnu::always_inline, gnu::hot]]
    ssize_t send(const void* buf, size_t len, const sockaddr_in& peer, int flags = 0)
    {
        return ::sendto(m_sock, buf, len, flags,
                        reinterpret_cast<const sockaddr*>(&peer), sizeof(peer));
    }

    [[nodiscard, gnu::always_inline, gnu::hot]]
    ssize_t recv(void* buf, size_t len, sockaddr_in& peer, int flags = 0)
    {
        socklen_t peer_len = sizeof(peer);
        return ::recvfrom(m_sock, buf, len, flags,
                          reinterpret_cast<sockaddr*>(&peer), &peer_len);
    }

};

class UdpServer : public UdpBase
{
  public:
    UdpServer() = default;
    explicit UdpServer(std::string ip, std::uint16_t port) : m_ip{std::move(ip)}, m_port{port} {}

    auto start() -> std::expected<void, UdpError>
    {
        if (m_started)
            return {};

        if (auto res = create_socket(); !res)
            return res;

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = ::htons(m_port);
        addr.sin_addr.s_addr = ::inet_addr(m_ip.c_str());

        if (::bind(get_fd(), reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
        {
            return std::unexpected{UdpError::BIND_FAIL};
        }

        m_started = true;
        return {};
    }

    [[nodiscard, gnu::always_inline, gnu::hot]] 
    auto send(const void* buf, size_t len, const sockaddr_in& peer)
        -> std::expected<std::uint64_t, UdpError>
    {
        ssize_t result = UdpBase::send(buf, len, peer);
        if (result < 0) [[unlikely]]
            return std::unexpected(UdpError::SEND_FAIL);
        return static_cast<std::uint64_t>(result);
    }

    [[nodiscard, gnu::always_inline, gnu::hot]] 
    auto recv(void* buf, size_t len, sockaddr_in& peer)
        -> std::expected<std::uint64_t, UdpError>
    {
        ssize_t result = UdpBase::recv(buf, len, peer);
        if (result < 0) [[unlikely]]
            return std::unexpected(UdpError::RECV_FAIL);
        return static_cast<std::uint64_t>(result);
    }

private:
    std::string m_ip{};
    std::uint16_t m_port{};
    bool m_started{false};
};

class UdpClient : public UdpBase
{
  public:
    UdpClient() = default;

    explicit UdpClient(std::string ip, std::uint16_t port)
    {
        create_socket();
        m_peer.sin_family = AF_INET;
        m_peer.sin_port = ::htons(port);
        ::inet_pton(AF_INET, ip.c_str(), &m_peer.sin_addr);
    }

    auto bind_local(std::uint16_t port) -> std::expected<void, UdpError>
    {
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = ::htons(port);
        addr.sin_addr.s_addr = INADDR_ANY;
        if (::bind(get_fd(), reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
            return std::unexpected(UdpError::BIND_FAIL);
        return {};
    }

    [[nodiscard, gnu::always_inline, gnu::hot]]
    auto recv(void* buf, size_t len, sockaddr_in& peer)
        -> std::expected<std::size_t, UdpError>
    {
        ssize_t result = UdpBase::recv(buf, len, peer);
        if (result < 0) [[unlikely]]
            return std::unexpected(UdpError::RECV_FAIL);
        return static_cast<std::size_t>(result);
    }

    template <class Self>
    sockaddr_in get_sockaddr_in(this Self&& self)
    {
        return std::forward<Self>(self).m_peer;
    }

private:
    sockaddr_in m_peer;
};


} // End namespace fiah
