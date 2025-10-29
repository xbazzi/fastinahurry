#pragma once
#include <cstdint>

#pragma pack(push, 1)
struct MarketData {
    uint64_t seq_num;
    char symbol[8];
    double bid;
    double ask;
    uint64_t timestamp_ns;
};
#pragma pack(pop)
