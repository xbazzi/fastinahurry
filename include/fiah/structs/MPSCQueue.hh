#pragma once

#include <atomic>
#include <climits>
#include <memory>
#include <new>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <bit>
#include <immintrin.h>
#include <cmath>


#include "fiah/utils/Types.hh"

namespace fiah
{



template <sz_t SIZE>
concept size_pow2 = std::popcount(SIZE) == 1;

template <class T>
concept small_T = sizeof(T) <= sizeof(sz_t);

/// @brief Multi-producer, single-consumer queue for copy-constructible types.
/// @note This queue uses a flag instead of a sequence counter,
///       which results in increased tail latency.
///       It also does not support moving on insertion.
/// @tparam T 
/// @tparam SIZE 
/// @todo make specialization with atomic data for small T
template <class T, sz_t SIZE>
    requires size_pow2<SIZE> && 
    std::is_nothrow_copy_constructible_v<T> &&
    std::is_nothrow_move_assignable_v<T>
class MPSCQueue
{
public:
    #if defined(__cpp_lib_hardware_interference_size)
    using CacheLine = std::integral_constant<sz_t, std::hardware_destructive_interference_size>;
    #else
    using CacheLine = std::integral_constant<sz_t, 64ULL>;
    #endif

    static constexpr sz_t EXPECTED_CACHE_LINE_BYTES{sizeof(sz_t) * 8ULL};
    static constexpr sz_t MASK{SIZE - 1};

    static_assert(CacheLine::value == EXPECTED_CACHE_LINE_BYTES);

    struct alignas(CacheLine::value) AlignedAtomic
    {
        std::atomic<sz_t> val{};
    };
    static_assert(std::atomic<sz_t>::is_always_lock_free, "Atomic type is not lock-free.");

    struct alignas(CacheLine::value) Slot
    {
        alignas(T) std::byte data[sizeof(T)]{};
        std::atomic<bool> ready{};

        const T* ptr() const noexcept
        {
            return std::launder(reinterpret_cast<const T*>(data));
        }

        T* ptr() noexcept
        {
            return std::launder(reinterpret_cast<T*>(data));
        }
    };

    MPSCQueue() noexcept;
    bool try_push(const T& in) noexcept;
    bool try_pop(T& out) noexcept;

protected:
    
private:
    AlignedAtomic m_head;
    AlignedAtomic m_tail;
    Slot m_array[SIZE];
};

template <class T, sz_t SIZE>
        requires size_pow2<SIZE> && 
    std::is_nothrow_copy_constructible_v<T> &&
    std::is_nothrow_move_assignable_v<T>
MPSCQueue<T, SIZE>::MPSCQueue() noexcept
    : m_head{0},
      m_tail{0}
{
}

template <class T, sz_t SIZE>
        requires size_pow2<SIZE> && 
    std::is_nothrow_copy_constructible_v<T> &&
    std::is_nothrow_move_assignable_v<T>
[[nodiscard]] [[gnu::always_inline]]
inline bool MPSCQueue<T, SIZE>::try_push(const T& in) noexcept
{
    // we might need acquire since we have multiple producers
    auto tail = m_tail.val.load(std::memory_order_relaxed);
    sz_t backoff_counter{1};
    for (;;)
    {
        const auto head = m_head.val.load(std::memory_order_acquire);
        if (tail - head >= SIZE)
            return false;
        
        if (m_tail.val.compare_exchange_weak(
                tail,
                tail + 1,
                std::memory_order_relaxed,
                std::memory_order_relaxed)
            )
            break;

        for (auto _{0uz}; _ < backoff_counter; ++_)
            _mm_pause();
        backoff_counter = std::min(backoff_counter << 1, sz_t(64));
    }

    auto& slot = m_array[tail & MASK];
    std::construct_at(std::launder(reinterpret_cast<T*>(slot.data)), in);
    slot.ready.store(true, std::memory_order_release);
    return true;
}

template <class T, sz_t SIZE>
        requires size_pow2<SIZE> && 
    std::is_nothrow_copy_constructible_v<T> &&
    std::is_nothrow_move_assignable_v<T>
[[nodiscard]] [[gnu::always_inline]]
inline bool MPSCQueue<T, SIZE>::try_pop(T& out) noexcept
{
    const auto head = m_head.val.load(std::memory_order_relaxed);
    auto& slot = m_array[head & MASK];
    auto ready = slot.ready.load(std::memory_order_acquire);
    if (!ready)
        return false;

    T* val = slot.ptr();
    out = std::move(*val);

    std::destroy_at(val);
    slot.ready.store(false, std::memory_order_release);
    m_head.val.store(head + 1, std::memory_order_release);
    return true;
}


} // End namespace fiah



























