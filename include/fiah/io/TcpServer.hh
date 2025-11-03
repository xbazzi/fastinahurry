#pragma once

#include <cstdint>
#include <expected>

#include "fiah/io/Tcp.hh"
#include "fiah/utils/Logger.hh"
#include "fiah/io/TcpError.hh"

namespace fiah::io {

/// @brief TCP server implementation using POSIX internet sockets.
///        There is no Windows support; currently or ever.
/// @attention Guaranteed to not throw
class TcpServer : public Tcp
{
private:
    static constexpr std::uint16_t MAX_LISTEN_NUM{10};
    static inline utils::Logger<TcpServer>& m_logger{utils::Logger<TcpServer>::get_instance("TcpServer")};
public:
    TcpServer() = default;
    explicit TcpServer(const std::string&, std::uint16_t) noexcept;
    explicit TcpServer(std::string&&, std::uint16_t);

    std::expected<void,        TcpError> start();
    std::expected<SocketRAII,  TcpError> accept_client();
    std::expected<std::uint64_t, TcpError> send(SocketRAII&, const void*, size_t);
    std::expected<std::uint64_t, TcpError> recv(SocketRAII&, void*, size_t);
};
} // End namespace fiah::io