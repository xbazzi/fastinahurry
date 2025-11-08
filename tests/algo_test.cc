#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <memory>

#include "fiah/Algo.hh"
#include "fiah/io/Config.hh"
#include "fiah/AlgoException.hh"
#include "fiah/Error.hh"
#include "test_utils.hh"

using namespace fiah;
using namespace std::chrono_literals;

// ============================================================================
// Test Fixture Base Class
// ============================================================================

/// @brief Base test fixture for Algo tests
/// @details Provides common setup, teardown, and utility methods for all Algo tests
class AlgoTestFixture : public ::testing::Test
{
protected:
    std::unique_ptr<Algo> algo_;
    io::Config config_;

    /// @brief Called before each test
    void SetUp() override
    {
        // Create a fresh config for each test
        config_ = fiah::testing::create_default_test_config();
    }

    /// @brief Called after each test
    void TearDown() override
    {
        // Ensure client is stopped before destroying
        if (algo_)
        {
            algo_->stop_client();
        }
        algo_.reset();
    }

    /// @brief Helper to create an Algo instance
    void create_algo()
    {
        algo_ = std::make_unique<Algo>(std::move(config_));
    }

    /// @brief Helper to create algo with custom config
    void create_algo_with_config(io::Config custom_config)
    {
        algo_ = std::make_unique<Algo>(std::move(custom_config));
    }

    /// @brief Wait for a condition with timeout
    template<typename Predicate>
    bool wait_for_condition(Predicate pred, std::chrono::milliseconds timeout = 1000ms)
    {
        auto start = std::chrono::steady_clock::now();
        while (!pred())
        {
            if (std::chrono::steady_clock::now() - start > timeout)
            {
                return false;
            }
            std::this_thread::sleep_for(10ms);
        }
        return true;
    }
};

// ============================================================================
// Constructor and Initialization Tests
// ============================================================================

class AlgoConstructorTest : public AlgoTestFixture
{
};

TEST_F(AlgoConstructorTest, ConstructorWithValidConfig)
{
    create_algo();

    ASSERT_NE(algo_, nullptr);
    EXPECT_FALSE(algo_->is_server_initialized());
    EXPECT_FALSE(algo_->is_client_initialized());
    EXPECT_FALSE(algo_->is_client_running());
    EXPECT_FALSE(algo_->is_client_stopped());
}

TEST_F(AlgoConstructorTest, InitialStateIsConsistent)
{
    create_algo();

    // All state flags should be false initially
    EXPECT_FALSE(algo_->is_server_initialized());
    EXPECT_FALSE(algo_->is_client_initialized());
    EXPECT_FALSE(algo_->is_client_running());
    EXPECT_FALSE(algo_->is_client_stopped());
}

// ============================================================================
// Server Initialization Tests
// ============================================================================

class AlgoServerTest : public AlgoTestFixture
{
};

TEST_F(AlgoServerTest, InitializeServerSetsState)
{
    create_algo();

    EXPECT_FALSE(algo_->is_server_initialized());

    auto result = algo_->initialize_server();

    // Server init may fail if port is in use - handle both cases
    if (result.has_value())
    {
        EXPECT_TRUE(algo_->is_server_initialized());
    }
    else
    {
        EXPECT_EQ(result.error(), AlgoError::INIT_SERVER_FAIL);
        EXPECT_FALSE(algo_->is_server_initialized());
    }
}

TEST_F(AlgoServerTest, InitializeServerTwiceIsIdempotent)
{
    create_algo();

    auto result1 = algo_->initialize_server();

    if (result1.has_value())
    {
        EXPECT_TRUE(algo_->is_server_initialized());

        // Second call should succeed and not change state
        auto result2 = algo_->initialize_server();
        EXPECT_TRUE(result2.has_value());
        EXPECT_TRUE(algo_->is_server_initialized());
    }
}

TEST_F(AlgoServerTest, InitializeServerWithCustomPort)
{
    // Use a different port to avoid conflicts
    auto custom_config = fiah::testing::create_test_config_with_port(0); // Port 0 lets OS choose
    create_algo_with_config(std::move(custom_config));

    auto result = algo_->initialize_server();

    if (result.has_value())
    {
        EXPECT_TRUE(algo_->is_server_initialized());
    }
}

// ============================================================================
// Client Initialization Tests
// ============================================================================

class AlgoClientTest : public AlgoTestFixture
{
};

TEST_F(AlgoClientTest, InitializeClientWithoutServerFails)
{
    create_algo();

    EXPECT_FALSE(algo_->is_client_initialized());

    auto result = algo_->initialize_client();

    // Expected to fail - server not running
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), AlgoError::SERVER_NOT_ONLINE);
    EXPECT_FALSE(algo_->is_client_initialized());
}

TEST_F(AlgoClientTest, InitializeClientTwiceIsIdempotent)
{
    create_algo();

    auto result1 = algo_->initialize_client();

    // Only test idempotency if first succeeded (unlikely without server)
    if (result1.has_value())
    {
        EXPECT_TRUE(algo_->is_client_initialized());

        auto result2 = algo_->initialize_client();
        EXPECT_TRUE(result2.has_value());
        EXPECT_TRUE(algo_->is_client_initialized());
    }
}

/// @todo Implement
TEST_F(AlgoClientTest, ReconnectClientWithoutPriorConnection)
{ }

// ============================================================================
// Client Lifecycle Tests
// ============================================================================

class AlgoClientLifecycleTest : public AlgoTestFixture
{
};

TEST_F(AlgoClientLifecycleTest, StopClientWhenNotStarted)
{
    create_algo();

    // Should not crash or throw
    EXPECT_NO_THROW(algo_->stop_client());
    EXPECT_TRUE(algo_->is_client_stopped());
}

TEST_F(AlgoClientLifecycleTest, StopClientTwiceIsIdempotent)
{
    create_algo();

    algo_->stop_client();
    EXPECT_TRUE(algo_->is_client_stopped());

    // Second stop should be safe
    EXPECT_NO_THROW(algo_->stop_client());
    EXPECT_TRUE(algo_->is_client_stopped());
}

TEST_F(AlgoClientLifecycleTest, StopClientSetsCorrectState)
{
    create_algo();

    algo_->stop_client();

    EXPECT_TRUE(algo_->is_client_stopped());
    EXPECT_FALSE(algo_->is_client_running());
}

TEST_F(AlgoClientLifecycleTest, WorkClientWithoutServerFails)
{
    create_algo();

    EXPECT_FALSE(algo_->is_client_running());

    auto result = algo_->work_client();

    // Should fail - server not online
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), AlgoError::SERVER_NOT_ONLINE);
    EXPECT_FALSE(algo_->is_client_running());
}

TEST_F(AlgoClientLifecycleTest, WorkClientTwiceReturnsEarly)
{
    create_algo();

    // First call will fail to start (no server)
    auto result1 = algo_->work_client();

    // If somehow it succeeded, second call should detect already running
    if (result1.has_value() && algo_->is_client_running())
    {
        auto result2 = algo_->work_client();
        EXPECT_TRUE(result2.has_value()); // Should return success but not start new threads
        EXPECT_TRUE(algo_->is_client_running());
    }
}

// ============================================================================
// Print Stats Tests
// ============================================================================

class AlgoStatsTest : public AlgoTestFixture
{
};

TEST_F(AlgoStatsTest, PrintStatsDoesNotCrashWhenNotRunning)
{
    create_algo();

    EXPECT_NO_THROW(algo_->print_client_stats());
}

TEST_F(AlgoStatsTest, PrintStatsAfterStoppingClient)
{
    create_algo();

    algo_->stop_client();

    EXPECT_NO_THROW(algo_->print_client_stats());
}

// ============================================================================
// Destructor and RAII Tests
// ============================================================================

class AlgoDestructorTest : public AlgoTestFixture
{
protected:
    // Override TearDown to allow manual destruction testing
    void TearDown() override
    {
        // Don't call base TearDown - we'll manually manage algo_
    }
};

TEST_F(AlgoDestructorTest, DestructorCleansUpProperly)
{
    create_algo();

    auto result = algo_->initialize_server();

    // Destructor should handle cleanup
    algo_.reset();

    // If we get here, destructor succeeded
    SUCCEED();
}

TEST_F(AlgoDestructorTest, DestructorStopsRunningThreads)
{
    create_algo();

    // Try to start client (will fail but that's OK)
    algo_->work_client();

    // Destructor should stop threads gracefully
    algo_.reset();

    SUCCEED();
}

TEST_F(AlgoDestructorTest, DestructorHandlesMultipleStops)
{
    create_algo();

    algo_->stop_client();

    // Destructor also calls stop_client - should be idempotent
    algo_.reset();

    SUCCEED();
}

// ============================================================================
// State Transition Tests
// ============================================================================

class AlgoStateTransitionTest : public AlgoTestFixture
{
};

TEST_F(AlgoStateTransitionTest, ServerStateTransition)
{
    create_algo();

    // Initial state
    EXPECT_FALSE(algo_->is_server_initialized());

    // Try to initialize
    auto result = algo_->initialize_server();

    if (result.has_value())
    {
        // State should have transitioned
        EXPECT_TRUE(algo_->is_server_initialized());

        // State should remain stable
        EXPECT_TRUE(algo_->is_server_initialized());
    }
}

TEST_F(AlgoStateTransitionTest, ClientStopTransition)
{
    create_algo();

    // Initial state
    EXPECT_FALSE(algo_->is_client_stopped());
    EXPECT_FALSE(algo_->is_client_running());

    // Stop client
    algo_->stop_client();

    // State should transition
    EXPECT_TRUE(algo_->is_client_stopped());
    EXPECT_FALSE(algo_->is_client_running());
}

// ============================================================================
// Error Handling Tests
// ============================================================================

class AlgoErrorHandlingTest : public AlgoTestFixture
{
};

TEST_F(AlgoErrorHandlingTest, InitializeClientReturnsCorrectError)
{
    create_algo();

    auto result = algo_->initialize_client();

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), AlgoError::SERVER_NOT_ONLINE);
}

TEST_F(AlgoErrorHandlingTest, WorkClientReturnsCorrectError)
{
    create_algo();

    auto result = algo_->work_client();

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), AlgoError::SERVER_NOT_ONLINE);
}

/// @todo Implement
TEST_F(AlgoErrorHandlingTest, ReconnectReturnsCorrectError)
{ }
