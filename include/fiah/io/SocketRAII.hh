#pragma once

#include <unistd.h>
#include <type_traits>

class SocketRAII
{
  private:
    int m_fd;

  public:
    SocketRAII() : m_fd{-1} {}
    explicit SocketRAII(int fd) : m_fd{fd}
    {
        static_assert(!std::is_copy_constructible_v<SocketRAII>);
        static_assert(!std::is_copy_assignable<SocketRAII>::value);
    }
    SocketRAII(const SocketRAII &) = delete;            // No copy ctor
    SocketRAII &operator=(const SocketRAII &) = delete; // No copy assg
    SocketRAII(SocketRAII &&other) noexcept(std::is_move_constructible_v<SocketRAII>) : m_fd{other.m_fd}
    {
        other.m_fd = -1;
    } // Move ctor
    SocketRAII &operator=(SocketRAII &&other) noexcept(std::is_move_assignable_v<SocketRAII>) // Move assg
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
    ~SocketRAII()
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
    SocketRAII &operator=(int newfd) noexcept
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
