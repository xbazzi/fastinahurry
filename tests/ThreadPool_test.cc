// clang-format off
#include "fiah/thread/ThreadPool.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>

#include "fiah/structs/SPSCQueue.hh"
#include "fiah/utils/Timer.hh"
#include "test_utils.hh"
// clang-format on

class ThreadPoolTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
        // Clean up
    }
};

TEST_F(ThreadPoolTest, ConstructMultipleThreads)
{
    std::size_t num_threads = 1 << 5;
    fiah::ThreadPool tp(num_threads);
    EXPECT_TRUE(tp.get_num_threads() == num_threads) << num_threads;
    TEST_COUT << "num_threads: " << num_threads << std::endl;
    EXPECT_FALSE(tp.get_num_active_tasks());
}