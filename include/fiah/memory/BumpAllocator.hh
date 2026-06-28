#pragma once

#include <new>
#include <cstddef>
#include <cstdlib>
#include <limits>
#include <type_traits>

#include "fiah/utils/Types.hh"
#include "fiah/memory/BumpArena.hh"

namespace fiah
{

[[noreturn]]
#if defined(__GNUC__) || defined(__clang__)
__attribute((cold, noinline))
#endif
inline void alloc_fail()
{
#if defined(__GNUC__) || defined(__clang__)
    __builtin_trap();
#else 
    std::abort();
#endif

}

template <class T>
class BumpAllocator
{
public:
    using value_type = T;

    template <class U>
    struct rebind { using other = BumpAllocator<U>; };

    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;
    using is_always_equal = std::false_type;

    constexpr BumpAllocator() noexcept = default;
    constexpr BumpAllocator(BumpArena* a) noexcept;
    template <class U> constexpr BumpAllocator(const BumpAllocator<U>& other) noexcept;
    template <class U> bool operator==(const BumpAllocator<U>& other) const noexcept;
    template <class U> bool operator!=(const BumpAllocator<U>& other) const noexcept;

    [[nodiscard]]
    constexpr T* allocate(sz_t n) noexcept;

    constexpr void deallocate(T* p, sz_t n) noexcept;

private:
    template <class> friend class BumpAllocator;
    BumpArena* m_arena;
};

template <class T>
constexpr BumpAllocator<T>::BumpAllocator(BumpArena* arena) noexcept
    : m_arena{arena}
{
}

template <class T>
    template <class U>
constexpr BumpAllocator<T>::BumpAllocator(const BumpAllocator<U>& other) noexcept
    : m_arena{other.m_arena}
{
}


template <class T> 
   template <class U> 
bool BumpAllocator<T>::operator==(const BumpAllocator<U>& other) const noexcept
{
    return m_arena == other.m_arena;

}

template <class T>
   template <class U> 
bool BumpAllocator<T>::operator!=(const BumpAllocator<U>& other) const noexcept
{
    return !(*this == other);
}

template <class T>
constexpr T* BumpAllocator<T>::allocate(sz_t n) noexcept
{
    if (!m_arena | (n > std::numeric_limits<sz_t>::max() / sizeof(T)))
        alloc_fail();

    auto ptr = static_cast<T*>(m_arena->allocate(n * sizeof(T), alignof(T)));
    if (!ptr)
        alloc_fail();
    return ptr;
}

template <class T>
constexpr void BumpAllocator<T>::deallocate(T*, sz_t) noexcept
{
    return; // no-op
}
} // End namespace fiah
