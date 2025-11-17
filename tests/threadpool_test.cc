#include "fiah/thread/ThreadPool.hh"
#include "test_utils.hh"
#include <gtest/gtest.h>
#include <memory>

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
    int num_threads = 1 << 13;
    fiah::thread::ThreadPool tp(num_threads);
    EXPECT_TRUE(tp.get_num_threads() == num_threads) << num_threads;
    TEST_COUT << "num_threads: " << num_threads << std::endl;
    EXPECT_FALSE(tp.get_num_active_tasks());
}