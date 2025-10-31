#pragma once

#include "utils/Types.hh"
#include <memory>
#include <list>


enum class OrderType 
{
    GoodTillCancel,
    FillAndKill,
    FillOrKill,
    GoodForDay,
    Market,
};

enum class Side
{
    Buy,
    Sell
};

class Order
{
private:
    OrderType _type;
    OrderId _id;
    Side _side;
    Price _price;
    Quantity _initial_quantity;
    Quantity _remaining_quantity;

public:
    Order() = delete;
    Order(OrderType, OrderId, Side, Price, Quantity);
    Order(OrderId, Side, Quantity);
};
using OrderPointer = std::shared_ptr<Order>;
using OrderPointers = std::list<OrderPointer>;