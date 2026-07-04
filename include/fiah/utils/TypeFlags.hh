#pragma once
#include <cstdint>
namespace fiah {
    template <std::size_t N>
    requires (N > 0)
    struct ReserveInitial { consteval ReserveInitial() noexcept = default; };

    template <std::size_t N>
    inline constexpr ReserveInitial<N> reserve_initial;
}
