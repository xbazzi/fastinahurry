#pragma once

#include <stdexcept>
#include <string>
#include <optional>
#include "io/TcpError.hh"

class AlgoException : public std::runtime_error
{
protected:
    std::optional<TcpError> m_tcp_error;

public:
    explicit AlgoException(const std::string& message)
        : std::runtime_error(message)
        , m_tcp_error(std::nullopt)
    {}

    AlgoException(const std::string& message, TcpError tcp_err)
        : std::runtime_error(message + " (TcpError: " + tcp_error_to_string(tcp_err) + ")")
        , m_tcp_error(tcp_err)
    {}

    std::optional<TcpError> get_tcp_error() const noexcept {
        return m_tcp_error;
    }

private:
    static std::string tcp_error_to_string(TcpError err) {
        switch (err) {
            case TcpError::BAD_SOCKET:   return "BAD_SOCKET";
            case TcpError::BIND_FAIL:    return "BIND_FAIL";
            case TcpError::LISTEN_FAIL:  return "LISTEN_FAIL";
            case TcpError::CONNECT_FAIL: return "CONNECT_FAIL";
            case TcpError::ACCEPT_FAIL:  return "ACCEPT_FAIL";
            case TcpError::SEND_FAIL:    return "SEND_FAIL";
            case TcpError::RECV_FAIL:    return "RECV_FAIL";
            case TcpError::INVALID_IP:   return "INVALID_IP";
            default:                     return "UNKNOWN";
        }
    }
};

class AlgoInitializationException : public AlgoException
{
public:
    explicit AlgoInitializationException(const std::string& message)
        : AlgoException("Initialization failed: " + message)
    {}

    AlgoInitializationException(const std::string& message, TcpError tcp_err)
        : AlgoException("Initialization failed: " + message, tcp_err)
    {}
};

class AlgoConfigException : public AlgoException
{
public:
    explicit AlgoConfigException(const std::string& message)
        : AlgoException("Configuration error: " + message)
    {}
};
