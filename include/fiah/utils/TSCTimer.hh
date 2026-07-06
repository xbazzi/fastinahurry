#pragma once

// C++ Includes
#include <cstdint>
#ifdef _WIN32
#incldue < intrin.h>
#else
#include <x86intrin.h>
#endif

#include "fiah/utils/Types.hh"
namespace fiah
{
class TSCTimer
{
  public:
    explicit TSCTimer(double dbTSCHz) : m_dbCyclesToMicros{1'000'000.0 / dbTSCHz}
    {
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
    double m_dbCyclesToMicros;
};
} // namespace fiah
