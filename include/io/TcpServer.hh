#pragma once

#include <cstdint>
#include <expected>

#include "io/Tcp.hh"
#include "utils/Logger.hh"
#include "io/TcpError.hh"

namespace io {

class TcpServer : public Tcp
{
private:
    static constexpr std::uint16_t MAX_LISTEN_NUM{10};
    utils::Logger& m_logger;
public:
    TcpServer() = default;
    explicit TcpServer(const std::string&, std::uint16_t);

    std::expected<void,        TcpError> start();
    std::expected<SocketRAII,  TcpError> accept_client();
    std::expected<std::uint64_t, TcpError> send(SocketRAII&, const void*, size_t);
    std::expected<std::uint64_t, TcpError> recv(SocketRAII&, void*, size_t);
};
} // End namespace io