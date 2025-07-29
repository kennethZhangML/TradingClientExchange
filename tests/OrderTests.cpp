#include <gtest/gtest.h>
#include "Order.hpp"

TEST(OrderBasic, ConstructorAndAccessors)
{
    Order o("AAPL", OrderSide::BUY, OrderType::LIMIT, 150.25, 100);
    EXPECT_EQ(o.getSymbol(),  "AAPL");
    EXPECT_EQ(o.getSide(),    OrderSide::BUY);
    EXPECT_EQ(o.getType(),    OrderType::LIMIT);
    EXPECT_EQ(o.getPrice(),   150.25);
    EXPECT_EQ(o.getQuantity(),100);
    EXPECT_TRUE(o.isActive());
}

TEST(OrderBasic, ModifyReduceCancel)
{
    Order o("MSFT", OrderSide::SELL, OrderType::LIMIT, 300.00, 50);

    o.modify(299.50, 40);
    EXPECT_EQ(o.getPrice(),   299.50);
    EXPECT_EQ(o.getQuantity(),40);

    o.reduceQuantity(15);
    EXPECT_EQ(o.getQuantity(),25);

    o.cancel();
    EXPECT_FALSE(o.isActive());
    EXPECT_EQ(o.getQuantity(),0);

    // Cancelling twice should be benign
    EXPECT_NO_THROW(o.cancel());
}

TEST(OrderValidation, InvalidConstruction)
{
    EXPECT_THROW(Order("AAPL", OrderSide::BUY, OrderType::LIMIT, 0.0,  10), std::invalid_argument);
    EXPECT_THROW(Order("AAPL", OrderSide::BUY, OrderType::LIMIT, 150,   0), std::invalid_argument);
    EXPECT_THROW(Order("",     OrderSide::BUY, OrderType::LIMIT, 150,  10), std::invalid_argument);
}

TEST(OrderValidation, InvalidModify)
{
    Order o("AAPL", OrderSide::BUY, OrderType::LIMIT, 150.0, 10);
    EXPECT_THROW(o.modify(-1.0, 10),               std::invalid_argument);
    EXPECT_THROW(o.modify(150.0,-5),               std::invalid_argument);
    o.cancel();
    EXPECT_THROW(o.modify(150.0, 5),               std::logic_error);
}
