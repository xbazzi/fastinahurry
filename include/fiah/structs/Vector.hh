#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <new>
#include <stdexcept>
#include <cassert>
#include <format>

#include "../utils/TypeFlags.hh"

namespace fiah
{

template <class T> class Vector
{
    static constexpr auto TSize = sizeof(T);
    static constexpr std::align_val_t TAlign{alignof(T)};
    static constexpr auto ExpansionMult = 2;

    T* m_begin{nullptr};
    T* m_end{nullptr};
    T* m_cap{nullptr};

    [[nodiscard]] static constexpr T* _allocate(std::size_t alloc_size) noexcept
    {
        assert(alloc_size != 0);
        return static_cast<T*>(::operator new(TSize * alloc_size, TAlign));
    }

    constexpr void _grow_capacity(std::size_t new_cap) noexcept
    {
        auto const new_arr = _allocate(new_cap);
        auto const curr_sz = size();
        auto i = curr_sz;

        while(i-- > 0)
            std::construct_at(new_arr + i, std::move_if_noexcept(m_begin[i]));

        std::destroy_n(m_begin, curr_sz);
        ::operator delete(m_begin, TAlign);
        m_begin = new_arr;
        m_end = m_begin + curr_sz;
        m_cap = m_begin + new_cap;
    }

    constexpr void _allocate_from_nullptr(std::size_t new_cap = 1) noexcept
    {
        assert(m_begin == nullptr); assert(m_end == nullptr); assert(m_cap == nullptr);
        m_end = m_begin = _allocate(new_cap);
        m_cap = m_begin + new_cap;
    }

    constexpr void _destroy_all() noexcept
    {
        if(not m_begin) return;
        std::destroy_n(m_begin, size());
        m_end = m_begin;
    }

    constexpr void _free() noexcept
    {
        ::operator delete(m_begin, TAlign);
        m_begin = m_cap = m_end = nullptr;
    }

    constexpr void _destroy_and_free() noexcept
    {
        _destroy_all();
        _free();
    }

public:

    using value_type = T;
    constexpr Vector() noexcept = default;

    template <std::size_t N>
    constexpr Vector(ReserveInitial<N>) noexcept
    {
        _allocate_from_nullptr(N);
    }

    constexpr Vector(Vector const& other) noexcept
    {
        _allocate_from_nullptr(other.capacity());
        m_end = std::uninitialized_copy_n(other.m_begin, other.size(), m_begin);
    }

    Vector& operator=(Vector const& other) noexcept
    {
        reserve(other.size());
        m_end = std::copy_n(other.m_begin, other.size(), m_begin);
        return *this;
    }

    constexpr Vector(Vector&& other) noexcept
        : m_begin(other.m_begin), 
          m_end(other.m_end),
          m_cap(other.m_cap)
    {
        other.m_cap = other.m_end = other.m_begin = nullptr;
    }

    Vector& operator=(Vector&& other) noexcept
    {
        _destroy_and_free();
        m_begin = other.m_begin; m_end = other.m_end; m_cap = other.m_cap;
        other.m_cap = other.m_end = other.m_begin = nullptr;
        return *this;
    }

    constexpr ~Vector() noexcept
    {
        _destroy_and_free();
    }

    T* begin()
    {
        return m_begin;
    }

    T* end()
    {
        return m_end;
    }

    template <class... Args> T& emplace_back(Args &&...args) noexcept
    {
        if(m_begin == nullptr) _allocate_from_nullptr();
        else if (m_end == m_cap) _grow_capacity(capacity() * ExpansionMult);
        return *std::construct_at(m_end++, std::forward<Args>(args)...);
    }

    constexpr void push_back(T const& element) noexcept
    {
        emplace_back(element);
    }

    constexpr void push_back(T&& element) noexcept
    {
        emplace_back(std::move(element));
    }

    constexpr T& at(std::size_t index)
    {
        if (index >= size()) throw std::out_of_range(std::format("Access at index {} within fiah::Vector of size {}.", index, size()));
        return m_begin[index];
    }

    constexpr T const& at(std::size_t index) const
    {
        if (index >= size()) throw std::out_of_range(std::format("Access at index {} within fiah::Vector of size {}.", index, size()));
        return m_begin[index];
    }

    [[nodiscard]] constexpr std::size_t size() const noexcept
    {
        return static_cast<std::size_t>(m_end - m_begin);
    }

    [[nodiscard]] constexpr std::size_t capacity() const noexcept
    {
        return m_cap - m_begin;
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return m_begin == m_end;
    }

    constexpr void reserve(std::size_t new_cap) noexcept
    {
        if(m_begin == nullptr) return _allocate_from_nullptr(new_cap);

        auto const current_cap = capacity();
        if(current_cap >= new_cap) return;

        _grow_capacity(new_cap);
    }

    constexpr void resize(std::size_t new_size) noexcept
    {
        if(m_begin == nullptr) {
            _allocate_from_nullptr(new_size);
            while(m_end not_eq m_cap) std::construct_at(m_end++);
            return;
        }

        auto const current_size = size();
        auto const new_end = m_begin + new_size;
        if(current_size >= new_size) {
            while(m_end not_eq new_end) std::destroy_at(--m_end);
            return;
        }

        if(capacity() < new_size) _grow_capacity(new_size);

        auto const num_default{new_size - current_size};
        while(m_end not_eq new_end)
            std::construct_at(m_end++);
    }

    constexpr void pop_back() noexcept
    {
        assert(not empty());
        std::destroy_at(--m_end);
    }

    constexpr void clear() noexcept
    {
        _destroy_all();
    }

    constexpr T& operator[](std::size_t pos) noexcept
    {
        assert(pos < size());
        return m_begin[pos];
    }

    constexpr T const& operator[](std::size_t pos) const noexcept
    {
        assert(pos < size());
        return m_begin[pos];
    }
};

} // namespace fiah
