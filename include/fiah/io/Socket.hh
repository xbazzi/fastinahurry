#pragma once

#include <unistd.h>
#include <type_traits>

class Socket
{
  private:
    int m_fd;

  public:
    Socket() : m_fd{-1} {}
    explicit Socket(int fd) : m_fd{fd}
    {
        static_assert(!std::is_copy_constructible_v<Socket>);
        static_assert(!std::is_copy_assignable<Socket>::value);
    }
    Socket(const Socket &) = delete;            // No copy ctor
    Socket &operator=(const Socket &) = delete; // No copy assg
    Socket(Socket &&other) noexcept(std::is_move_constructible_v<Socket>) : m_fd{other.m_fd}
    {
        other.m_fd = -1;
    } // Move ctor
    Socket &operator=(Socket &&other) noexcept(std::is_move_assignable_v<Socket>) // Move assg
    {
        if (this != &other)
        {
            if (m_fd >= 0)
                close(m_fd);
            m_fd = other.m_fd;
            other.m_fd = -1;
        }
        return *this;
    }
    ~Socket()
    {
        if (m_fd >= 0)
            close(m_fd);
    }

    // Allow use in system calls
    operator int() const noexcept
    {
        return m_fd;
    }

    // Assignment from raw socket
    Socket &operator=(int newfd) noexcept
    {
        if (m_fd >= 0)
            ::close(m_fd);
        m_fd = newfd;
        return *this;
    }

    [[nodiscard]] bool valid() const noexcept
    {
        return m_fd >= 0;
    }

    void reset(int newfd = -1) noexcept
    {
        if (m_fd >= 0)
            close(m_fd);
        m_fd = newfd;
    }
};
