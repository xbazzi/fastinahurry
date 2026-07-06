#include <gtest/gtest.h>
#include <memory>

#include "test_utils.hh"
#include "fiah/utils/SPSCLogger.hh"

using namespace fiah;

class LoggerNewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    SPSCLogger& m_logger{SPSCLogger::get_instance()};
};

TEST_F(LoggerNewTest, SingleProducer)
{
    using namespace std::chrono_literals;
    m_logger.error(std::source_location::current(), "No\n");
    m_logger.info(std::source_location::current(), "woman\n");
    m_logger.warn(std::source_location::current(), "no\n");
    m_logger.error(std::source_location::current(), "cry\n");
    m_logger.info(std::source_location::current(), "no\n");
    m_logger.warn(std::source_location::current(), "woman\n");
    m_logger.error(std::source_location::current(), "no\n");
    m_logger.info(std::source_location::current(), "cry\n");
    m_logger.warn(std::source_location::current(), ":')\n");

    std::this_thread::sleep_for(100ms);
}

TEST_F(LoggerNewTest, MultiProducer)
{
    using namespace std::chrono_literals;
    const auto log = [this](const int consumer_no) noexcept { 
        m_logger.info(std::source_location::current(), "consumer%d\n", consumer_no); 
        m_logger.warn(std::source_location::current(), "consumer%d\n", consumer_no); 
        m_logger.error(std::source_location::current(), "consumer%d\n", consumer_no); 
    };

    std::jthread(log, 1);
    std::jthread(log, 2);
    std::jthread(log, 3);
    std::jthread(log, 4);

    // 4 + this test's thread
    EXPECT_EQ(5UZ, m_logger.get_num_threads());

    std::this_thread::sleep_for(100ms);
}
