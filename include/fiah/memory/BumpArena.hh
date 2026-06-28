#pragma once

#include <cstdint>
#include <cstddef>
#include <memory>


#include "fiah/utils/Types.hh"

namespace fiah
{

class BumpArena
{
public:
    BumpArena(void* buff, sz_t size) noexcept;
    void* allocate(sz_t size, sz_t alignment) noexcept;
    sz_t used() noexcept;
    sz_t remaining() noexcept;
    bool full() noexcept;
    void reset() noexcept;

protected:
    
private:
    std::byte* m_begin;
    std::byte* m_curr;
    std::byte* m_end;
};

BumpArena::BumpArena(void* buff, sz_t size) noexcept
    : m_begin{static_cast<std::byte*>(buff)},
      m_curr{m_begin},
      m_end{m_begin + size}
{
}

[[nodiscard]]
void* BumpArena::allocate(sz_t size, sz_t alignment) noexcept
{
    void* ptr = m_curr;
    sz_t space = remaining();

    void* aligned = std::align(alignment, size, ptr, space);

    if (!aligned)
        return nullptr;

    m_curr = static_cast<std::byte*>(aligned) + size;
    return aligned;
}

sz_t BumpArena::used() noexcept
{
    return static_cast<sz_t>(m_curr - m_begin);
}

sz_t BumpArena::remaining() noexcept
{
    return static_cast<sz_t>(m_end - m_curr);
}

bool BumpArena::full() noexcept
{
    return m_curr == m_end;
}

void BumpArena::reset() noexcept
{
    m_curr = m_begin;
}
} // End namespace fiah