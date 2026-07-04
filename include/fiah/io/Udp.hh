#pragma once

// C++ Includes
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cstdint>
#include <expected>
#include <string>

// FastInAHurry Includes
#include "fiah/error/Error.hh"
#include "fiah/io/Socket.hh"

namespace fiah {

class UdpBase
{
protected:
    UdpBase(const UdpBase&) = delete;
    UdpBase& operator=(const UdpBase&) = delete;
    UdpBase(UdpBase&&) = default;
    UdpBase& operator=(UdpBase&&) = default;
    ~UdpBase() noexcept = default;

protected:
    Socket m_sock;

protected:
    UdpBase() = default;

    auto create_socket() -> std::expected<void, UdpError>
    {
        auto const fd = ::socket(AF_INET, SOCK_DGRAM, 0);
        if (fd < 0)
            return std::unexpected(UdpError::BAD_SOCKET);
        m_sock = fd;
        return {};
    }

    int get_fd() const noexcept { return static_cast<int>(m_sock); }

    [[nodiscard, gnu::always_inline]]
    ssize_t send(const void* buf, size_t len, const sockaddr_in& peer, int flags = 0)
    {
        return ::sendto(m_sock, buf, len, flags,
                        reinterpret_cast<const sockaddr*>(&peer), sizeof(peer));
    }

    [[nodiscard, gnu::always_inline]]
    ssize_t recv(void* buf, size_t len, sockaddr_in& peer, int flags = 0)
    {
        socklen_t peer_len = sizeof(peer);
        return ::recvfrom(m_sock, buf, len, flags,
                          reinterpret_cast<sockaddr*>(&peer), &peer_len);
    }

};

class UdpServer : private UdpBase
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

    [[gnu::always_inline]] 
    auto send(const void* buf, size_t len, const sockaddr_in& peer) -> ssize_t
    {
        return UdpBase::send(buf, len, peer);
    }

    [[nodiscard, gnu::always_inline]] 
    auto recv(void* buf, size_t len, sockaddr_in& peer) -> ssize_t
    {
        return UdpBase::recv(buf, len, peer);
    }

private:
    std::string m_ip{};
    std::uint16_t m_port{};
    bool m_started{false};
};

[[noreturn]]
#if defined(GNUC) || defined(clang)
attribute((cold, noinline))
#endif
inline void udp_fail()
{
#if defined(GNUC__) || defined(clang)
    __builtin_trap();
#else
    std::abort();
#endif
}

class UdpClient : private UdpBase
{
  public:
    UdpClient() = default;

    explicit UdpClient(std::string ip, std::uint16_t port)
    {
        if (not create_socket()) udp_fail();

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

    [[nodiscard, gnu::always_inline]]
    auto recv(void* buf, size_t len, sockaddr_in& peer) -> ssize_t
    {
        return UdpBase::recv(buf, len, peer);
    }

    sockaddr_in get_sockaddr_in(this const auto& self)
    {
        return self.m_peer;
    }

private:
    sockaddr_in m_peer;
};
} // End namespace fiah
