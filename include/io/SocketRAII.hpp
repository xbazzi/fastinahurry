#pragma once

#include <unistd.h>

class SocketRAII {
    int _fd{-1};
public:
    SocketRAII() = default;
    explicit SocketRAII(int fd) : _fd(fd) {}
    SocketRAII(const SocketRAII&) = delete; // No copy ctor
    SocketRAII& operator=(const SocketRAII&) = delete; // No copy assg
    SocketRAII(SocketRAII&& other) noexcept : _fd{other._fd} { other._fd = -1; } // Move ctor
    SocketRAII& operator=(SocketRAII&& other) noexcept // Move assg
    {
        if (this != &other) {
            if (_fd >= 0) close(_fd);
            _fd = other._fd;
            other._fd = -1;
        }
        return *this;
    }
    ~SocketRAII() { if (_fd >= 0) close(_fd); }

    // Allow use in system calls
    operator int() const noexcept { return _fd; }

    // Assignment from raw socket
    SocketRAII& operator=(int newfd) noexcept 
    {
        if (_fd >= 0) close(_fd);
        _fd = newfd;
        return *this;
    }
    [[nodiscard]] bool valid() const noexcept { return _fd >= 0; }
    void reset(int newfd = -1) noexcept { if (_fd >= 0) close(_fd); _fd = newfd; }
};
