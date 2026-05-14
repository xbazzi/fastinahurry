// clang-format off
#include <gtest/gtest.h>
#include "fiah/utils/Timer.hh"
#include "test_utils.hh"
#include <stdio.h>
#include <inttypes.h>
// clang-format on

class TimerTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    fiah::Timer<> m_timer;
};

TEST_F(TimerTest, JustForFun)
{
    const auto nanos = m_timer.elapsedNs();
    printf("%" PRIu64 "\n", nanos);
}
