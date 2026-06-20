// C++ Includes
#include <cstdint>
#ifdef _WIN32
#incldue < intrin.h>
#else
#include <x86intrin.h>
#endif

// Third Party Includes
#include <nlohmann/json.hpp>

// FastInAHurry Includes
#include "fiah/utils/Logger.hh"
#include "fiah/utils/Timer.hh"

namespace fiah
{
class TSCTimer
{
  public:
    explicit TSCTimer(double dbTSCHz) : m_dbCyclesToMicros{1'000'000.0 / dbTSCHz}
    {
        unsigned aux;
        m_u64TimeNow = __rdtscp(&aux);
    }

    void increment(std::uint64_t ns)
    {
        m_u64TimeNow += ns;
    }

    std::uint64_t now()
    {
        return __rdtsc();
    }

    double cyclesToMicros(std::uint64_t cycles) const
    {
        return static_cast<double>(cycles) * m_dbCyclesToMicros;
    }

    ~TSCTimer()
    {
    }

  protected:
  private:
    std::uint64_t m_u64TimeNow;
    double m_dbCyclesToMicros;
};
} // namespace fiah