#pragma once

// C++ Includes
#include <cstdint>
#include <cstring>
#include <expected>
#include <sys/socket.h>

// FastInAHurry Includes
#include "fiah/io/SocketRAII.hh"
#include "fiah/error/Error.hh"

namespace fiah {
class Udp
{
public:
    static constexpr std::uint8_t MAX_IP_ADDR_LEN{15};
    Udp() :  m_port{1337}, m_sock{}, m_running{}, m_ip{"127.0.0.1"} {}
    explicit Udp(char* ip, std::uint16_t port, int fd) 
        : m_port{port}, m_sock{fd}, m_running{} 
    {
        std::memcpy(m_ip, ip, MAX_IP_ADDR_LEN);
    }
    Udp(const Udp& other) = delete;
    Udp(Udp&& other) = default;
    Udp& operator=(const Udp& other) = delete;
    Udp& operator=(Udp&& other) = default;
    ~Udp() = default;

    auto send(char* buf, ssize_t size, int flags) -> std::expected<std::uint16_t, UdpError>
    {
        ssize_t res = ::send(m_sock, buf, size, flags);
        if (res < 0)
            return std::unexpected(UdpError::SEND_FAIL);
        return static_cast<std::uint16_t>(res);
    }

    auto create_socket() -> std::expected<void, UdpError>
    {
        int fd = ::socket(AF_INET, SOCK_DGRAM, 0);
        if (fd < 0)
            return std::unexpected(UdpError::BAD_SOCKET);
        m_sock = fd;
        return {};
    }

    int get_fd() const noexcept
    {
        return static_cast<int>(m_sock);
    }

    [[gnu::hot]] ssize_t send_data(const void *buf, size_t len, sockaddr addr, flags = 0)
    {
        return ::sendto(m_sock, buf, len, flags, addr, addr_size);
    }

    [[nodiscard]] [[gnu::hot]] ssize_t recv_data(void *buf, size_t len, int flags = 0)
    {
        return ::recvfrom(m_sock, buf, len, flags);
    }

protected:
    std::uint16_t m_port;
    SocketRAII m_sock;
    bool m_running;
    char m_ip[MAX_IP_ADDR_LEN];
};

template <class Derived>
class UDPServer : Udp<
{
public:
    UDPServer();
    ~UDPServer();

private:
    SocketRAII m_socket;
};
} // End namespace fiah