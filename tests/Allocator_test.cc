#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <list>

#include "test_utils.hh"
#include "fiah/utils/Types.hh"
#include "fiah/memory/BumpAllocator.hh"
#include "fiah/memory/BumpArena.hh"

using namespace fiah;

class AllocatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

TEST_F(AllocatorTest, VectorValidAllocation)
{
    std::byte buff[1 << 5];
    BumpArena arena{buff, 1 << 5};
    BumpAllocator<int> int_alloc{&arena};
    std::vector<int, BumpAllocator<int>> v(int_alloc);
    sz_t valid_size = (1 << 5) / sizeof(int);
    v.resize(valid_size);
    EXPECT_EQ(v.size(), valid_size);
}

TEST_F(AllocatorTest, VectorInvalidAllocation)
{
    std::byte buff[1 << 5];
    BumpArena arena{buff, 1 << 5};
    BumpAllocator<int> int_alloc{&arena};
    std::vector<int, BumpAllocator<int>> v(int_alloc);
    sz_t invalid_size = (1<< 5) / sizeof(int) + 1;
    EXPECT_DEATH(v.resize(invalid_size), "");
}

TEST_F(AllocatorTest, ArenaInvalid)
{
    std::byte* buff{nullptr};
    BumpArena arena{buff, 1 << 5};
    BumpAllocator<int> int_alloc{&arena};
    std::vector<int, BumpAllocator<int>> v(int_alloc);
    EXPECT_DEATH(v.resize(1), "");
}

TEST_F(AllocatorTest, ListAllocation)
{
    constexpr sz_t size_of_int_node = 8 * 3; 
    constexpr sz_t size = size_of_int_node * 5;
    std::byte buff[size];
    BumpArena arena{buff, size};
    BumpAllocator<int> int_alloc{&arena};
    std::list<int, decltype(int_alloc)> l(int_alloc);
    EXPECT_EQ(l.size(), 0);
    
    l.push_back(1);
    l.push_back(2);
    l.push_back(3);
    l.push_back(4);
    l.push_back(5);
    EXPECT_EQ(l.size(), 5);
    l.remove(1);
    EXPECT_EQ(l.size(), 4);
    l.remove(1);
    EXPECT_EQ(l.size(), 4);
    l.remove(2);
    EXPECT_EQ(l.size(), 3);
    l.remove(3);
    EXPECT_EQ(l.size(), 2);
    l.remove(4);
    EXPECT_EQ(l.size(), 1);
    l.remove(5);
    EXPECT_EQ(l.size(), 0);

    EXPECT_DEATH(l.push_back(6), "");
}