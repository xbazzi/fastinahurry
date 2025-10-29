#pragma once

#include <Order.hh>

class Orderbook
{
private:
    struct OrderEntry
    {
        OrderPointer _order{ nullptr };
        OrderPointers::iterator _location;
    };

    struct LevelData
    {
        Quantity _qty{};
        Quantity _count{};
        enum class Action
        {
            Add,
            Remove,
            Match,
        };

    };
};