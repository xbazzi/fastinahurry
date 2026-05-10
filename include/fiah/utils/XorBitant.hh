#include <cstdint>
#include <limits>

class XorBitant
{
  private:
    constexpr static inline std::uint64_t MAGIC_MULTIPLIER = 0x9E3779B97F4A7C15ULL;
    constexpr static inline std::uint64_t MAGIC_SEED = 0xDEAD'BEEF'BABE'CAFEULL;
    std::uint64_t m_state{MAGIC_SEED};

  public:
    using result_type = std::uint32_t;

    explicit XorBitant(std::uint64_t seed = MAGIC_SEED) : m_state(seed)
    {
    }

    // Required: min value
    static constexpr result_type min()
    {
        return 0;
    }

    // Required: max value
    static constexpr result_type max()
    {
        return std::numeric_limits<std::uint32_t>::max();
    }

    // Generate next random number
    result_type operator()()
    {
        std::uint64_t x = m_state;
        x ^= x << 13;
        x ^= x >> 7;
        x ^= x << 5;
        m_state = x;
        return static_cast<result_type>(x * MAGIC_MULTIPLIER);
    }
};
