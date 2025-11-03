#pragma once

// C++ Includes
#include <cstdint>

// Third Party Includes

// FastInAHurry Includes
#include "fiah/structs/SPSCQueue.hh"
#include "fiah/market/MarketData.hh"

namespace fiah::structs {

///
/// @brief General purpose structs
/// @todo remove magic numbers
///

/// @brief Market signal
struct Signal
{
    char symbol[8];
    enum class Type : std::uint8_t
    {
        BUY,
        SELL,
        HOLD
    } type;
    double price;
    uint64_t quantity;
    uint64_t timestamp_ns;
};

struct Order
{
    char symbol[8];
    enum class Side : std::uint8_t
    {
        BUY,
        SELL
    } side;
    double price;
    uint64_t quantity;
    uint64_t order_id;
    uint64_t timestamp_ns;
};

struct Task
{
    enum class Type
    {
        MarketData,
        Signal,
        Order,
        Risk
    };
    Type type;
    union {
        MarketData market_data;
        Signal signal;
        Order order;
    };
};

struct Worker
{
    std::thread thread;
    structs::SPSCQueue<Task, 1024> queue;
    std::atomic<bool> running{true};
    int cpu_affinity;
};


} // End namespace fiah::structs