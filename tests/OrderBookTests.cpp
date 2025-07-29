#include <gtest/gtest.h>
#include "OrderBook.hpp"

using std::make_shared;

TEST(OrderBookBasic, BestBidAsk)
{
    OrderBook book("AAPL");
    auto b1 = make_shared<Order>("AAPL", OrderSide::BUY,  OrderType::LIMIT, 149.0, 100);
    auto s1 = make_shared<Order>("AAPL", OrderSide::SELL, OrderType::LIMIT, 150.0, 200);

    book.addOrder(b1);
    book.addOrder(s1);

    ASSERT_NE(book.getBestBid(), nullptr);
    ASSERT_NE(book.getBestAsk(), nullptr);
    EXPECT_DOUBLE_EQ(book.getBestBid()->getPrice(), 149.0);
    EXPECT_DOUBLE_EQ(book.getBestAsk()->getPrice(), 150.0);
}

TEST(OrderBookMatch, LimitCrossFullAndPartial)
{
    OrderBook book("AAPL");
    auto buy  = make_shared<Order>("AAPL", OrderSide::BUY,  OrderType::LIMIT, 150.00, 150);
    auto sell = make_shared<Order>("AAPL", OrderSide::SELL, OrderType::LIMIT, 149.50, 100);

    book.addOrder(buy);
    book.addOrder(sell);

    // Should trade 100, leave 50 on the bid
    auto trades = book.match();
    EXPECT_EQ(trades.size(), 1);
    EXPECT_EQ(buy->getQuantity(),  50);
    EXPECT_EQ(sell->isActive(),    false);
    EXPECT_EQ(book.getSellOrders().size(), 0);
}

TEST(OrderBookMatch, MarketOrders)
{
    OrderBook book("AAPL");
    auto ask = make_shared<Order>("AAPL", OrderSide::SELL, OrderType::LIMIT, 151.0, 75);
    book.addOrder(ask);

    auto mkt = make_shared<Order>("AAPL", OrderSide::BUY,  OrderType::MARKET, 0.0, 60);
    book.addOrder(mkt);

    auto trades = book.match();
    EXPECT_EQ(trades.size(), 1);
    EXPECT_EQ(ask->getQuantity(), 15); // 75 - 60
}

TEST(OrderBookOps, CancelAndModify)
{
    OrderBook book("AAPL");
    auto bid = make_shared<Order>("AAPL", OrderSide::BUY, OrderType::LIMIT, 148.0, 40);
    int id   = book.addOrder(bid);

    // modify quantity
    EXPECT_TRUE(book.modifyOrder(id, std::nullopt, 60));
    EXPECT_EQ(bid->getQuantity(), 60);

    // cancel
    EXPECT_TRUE(book.removeOrder(id));
    EXPECT_FALSE(bid->isActive());
    EXPECT_EQ(book.getBuyOrders().size(), 0);
}
