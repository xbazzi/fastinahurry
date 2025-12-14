/// @file test.cc
/// @brief Main test file - GTest entry point and basic sanity tests
///
/// This file serves as the main entry point for the test suite.
/// Specific test suites are organized in separate *_test.cc files:
/// - algo_test.cc: Comprehensive Algo class tests with fixtures
/// - Add more test files here following the same pattern
///
/// Test files use custom test fixtures and utilities defined in test_utils.hh

#include <gtest/gtest.h>
#include <string>

// ============================================================================
// Basic Sanity Tests
// ============================================================================

/// @brief Verify test framework is working
TEST(SanityTest, BasicAssertion)
{
    EXPECT_EQ(2 + 2, 4);
    EXPECT_TRUE(true);
    EXPECT_FALSE(false);
}

/// @brief Verify GTest comparison operators
TEST(SanityTest, ComparisonOperators)
{
    EXPECT_LT(1, 2);
    EXPECT_GT(3, 2);
    EXPECT_LE(2, 2);
    EXPECT_GE(2, 2);
    EXPECT_NE(1, 2);
}

/// @brief Verify GTest string comparisons
TEST(SanityTest, StringComparison)
{
    std::string str1 = "hello";
    std::string str2 = "hello";
    std::string str3 = "world";

    EXPECT_EQ(str1, str2);
    EXPECT_NE(str1, str3);
    EXPECT_STREQ("test", "test");
}

/// Auto generated doc
//
// When adding new tests, follow these best practices:
//
// 1. File Naming:
//    - Use *_test.cc for test files (e.g., algo_test.cc, config_test.cc)
//    - Use descriptive names that match the class/module being tested
//
// 2. Test Fixture Naming:
//    - Format: <ClassName><Feature>Test (e.g., AlgoServerTest, AlgoClientTest)
//    - Inherit from ::testing::Test for fixtures
//    - Use fixtures to share setup/teardown logic
//
// 3. Test Naming:
//    - Format: TEST_F(FixtureName, DescriptiveTestName)
//    - Use descriptive names that explain what is being tested
//    - Examples: InitializeServerSetsState, StopClientTwiceIsIdempotent
//
// 4. Test Organization:
//    - Group related tests using the same fixture class
//    - Use comments to separate test sections
//    - Keep tests focused and testing one thing
//
// 5. Custom Test Utilities:
//    - Place reusable test helpers in test_utils.hh
//    - Use TestConfigBuilder for creating test configurations
//    - Add custom matchers and assertions as needed
//
// Example structure:
//
//   class MyClassTest : public ::testing::Test {
//   protected:
//       void SetUp() override { /* common setup */ }
//       void TearDown() override { /* common cleanup */ }
//       // Helper methods and member variables
//   };
//
//   TEST_F(MyClassTest, SpecificBehaviorTest) {
//       // Arrange
//       // Act
//       // Assert
//   }
//
