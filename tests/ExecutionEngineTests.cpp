#include <gtest/gtest.h>
#include "ExecutionEngine.hpp"

using std::make_shared;

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------
static auto makeLimit(const std::string& sym, OrderSide s,
                      double px, int qty)
{
    return make_shared<Order>(sym, s, OrderType::LIMIT, px, qty);
}
static auto makeMkt(const std::string& sym, OrderSide s, int qty)
{
    return make_shared<Order>(sym, s, OrderType::MARKET, 0.0, qty);
}

// -----------------------------------------------------------------------------
// Suite 1 : basic life‑cycle & routing
// -----------------------------------------------------------------------------
TEST(EngineLifecycle, EnsureBookAndSubmit)
{
    ExecutionEngine eng;
    eng.ensureBook("AAPL");
    ASSERT_NE(eng.getBook("AAPL"), nullptr);

    auto bid = makeLimit("AAPL", OrderSide::BUY, 150.0, 10);
    int id   = eng.submit(bid);

    EXPECT_EQ(bid->getOrderId(), id);
    EXPECT_EQ(eng.getBook("AAPL")->getBuyOrders().size(), 1);
}

TEST(EngineLifecycle, MultiSymbolRouting)
{
    ExecutionEngine eng;

    auto aBid = makeLimit("AAPL", OrderSide::BUY, 100, 10);
    auto mAsk = makeLimit("MSFT", OrderSide::SELL, 200, 10);

    eng.submit(aBid);
    eng.submit(mAsk);

    EXPECT_EQ(eng.getBook("AAPL")->getBuyOrders().size(), 1);
    EXPECT_EQ(eng.getBook("MSFT")->getSellOrders().size(), 1);
}

// -----------------------------------------------------------------------------
// Suite 2 : trade callback scenarios
// -----------------------------------------------------------------------------
class TradeCountFixture : public ::testing::Test {
protected:
    void SetUp() override {
        eng.setTradeHandler([this](const ExecutionEngine::Trade& t) {
            trades.push_back(t);
        });
    }
    ExecutionEngine eng;
    std::vector<ExecutionEngine::Trade> trades;
};

TEST_F(TradeCountFixture, LimitAgainstLimit)
{
    auto buy  = make_shared<Order>("AAPL", OrderSide::BUY,
                                   OrderType::LIMIT, 150, 30);
    auto sell = make_shared<Order>("AAPL", OrderSide::SELL,
                                   OrderType::LIMIT, 149.5, 25);

    int buyId  = eng.submit(buy);
    int sellId = eng.submit(sell);

    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0].buyId , buyId );
    EXPECT_EQ(trades[0].sellId, sellId);
}


TEST_F(TradeCountFixture, MarketAgainstBook)
{
    eng.submit(makeLimit("AAPL", OrderSide::SELL, 150, 40));
    eng.submit(makeMkt  ("AAPL", OrderSide::BUY ,     35));

    EXPECT_EQ(trades.size(), 1);
}

TEST_F(TradeCountFixture, NoTradeWhenSpread)
{
    eng.submit(makeLimit("AAPL", OrderSide::BUY , 149.0, 100));
    eng.submit(makeLimit("AAPL", OrderSide::SELL, 151.0, 100));
    EXPECT_TRUE(trades.empty());
}

// -----------------------------------------------------------------------------
// Suite 3 : cancel / modify through engine
// -----------------------------------------------------------------------------
TEST(EngineCancelModify, CancelUpdatesMaps)
{
    ExecutionEngine eng;
    auto o = makeLimit("AAPL", OrderSide::BUY, 150, 20);
    int id = eng.submit(o);

    EXPECT_TRUE(eng.cancel(id));
    EXPECT_FALSE(o->isActive());
    EXPECT_EQ(eng.getBook("AAPL")->getBuyOrders().size(), 0);
}

TEST(EngineCancelModify, ModifyTriggersRematch)
{
    ExecutionEngine eng;
    int tradeCnt = 0;
    eng.setTradeHandler([&](const ExecutionEngine::Trade&){ ++tradeCnt; });

    auto bid = makeLimit("AAPL", OrderSide::BUY, 149, 50);
    auto ask = makeLimit("AAPL", OrderSide::SELL, 151, 50);
    eng.submit(bid);
    eng.submit(ask);

    eng.modify(bid->getOrderId(), 152.0, std::nullopt);
    EXPECT_EQ(tradeCnt, 1);
}

// -----------------------------------------------------------------------------
// Suite 4 : risk checks
// -----------------------------------------------------------------------------
TEST(EngineRisk, RejectTooLarge)
{
    ExecutionEngine eng;
    eng.setMaxOrderQty(99);
    EXPECT_THROW(
        eng.submit(makeLimit("AAPL", OrderSide::BUY, 150, 100)),
        std::invalid_argument);
}

TEST(EngineRisk, AcceptAtLimit)
{
    ExecutionEngine eng;
    eng.setMaxOrderQty(100);
    EXPECT_NO_THROW(
        eng.submit(makeLimit("AAPL", OrderSide::SELL, 150, 100)));
}

// -----------------------------------------------------------------------------
// Suite 5 : id lookup integrity
// -----------------------------------------------------------------------------
TEST(EngineIdMap, BookForOrder)
{
    ExecutionEngine eng;
    auto o1 = makeLimit("AAPL", OrderSide::BUY, 100, 10);
    auto o2 = makeLimit("MSFT", OrderSide::BUY, 200, 10);

    int id1 = eng.submit(o1);
    int id2 = eng.submit(o2);

    EXPECT_NE(eng.getBook("AAPL"), eng.getBook("MSFT"));
    EXPECT_TRUE( eng.cancel(id1));
    EXPECT_TRUE( eng.cancel(id2));
    EXPECT_FALSE(eng.cancel(id1));
}

// -----------------------------------------------------------------------------
// Suite 6 : edge‑cases & regressions
// -----------------------------------------------------------------------------
TEST(EngineEdge, CancelNonexistent)
{
    ExecutionEngine eng;
    EXPECT_FALSE(eng.cancel(12345));
}

TEST(EngineEdge, ModifyNonexistent)
{
    ExecutionEngine eng;
    EXPECT_FALSE(eng.modify(67890, 100.0, 10));
}

TEST(EngineEdge, MarketOrderNoLiquidity)
{
    ExecutionEngine eng;
    int calls = 0;
    eng.setTradeHandler([&](const ExecutionEngine::Trade&){ ++calls; });

    eng.submit(makeMkt("AAPL", OrderSide::BUY, 50));
    EXPECT_EQ(calls, 0);
    EXPECT_EQ(eng.getBook("AAPL")->getBuyOrders().size(), 1);
}

TEST(EngineEdge, ModifyToZeroQtyCancels)
{
    ExecutionEngine eng;
    auto o = makeLimit("AAPL", OrderSide::BUY, 100, 20);
    int id = eng.submit(o);

    eng.modify(id, std::nullopt, 0);
    EXPECT_FALSE(o->isActive());
    EXPECT_EQ(eng.getBook("AAPL")->getBuyOrders().size(), 0);
}

// -----------------------------------------------------------------------------
// Suite 7 : multi‑step trade simulation
// -----------------------------------------------------------------------------
TEST(EngineScenario, MultiStep)
{
    ExecutionEngine eng;
    int trades = 0;
    eng.setTradeHandler([&](const ExecutionEngine::Trade&){ ++trades; });

    // ── step 1 : build the book (no trade, wide spread) ──────────────────
    eng.submit(makeLimit("AAPL", OrderSide::BUY , 149.0, 50));   // id 0
    eng.submit(makeLimit("AAPL", OrderSide::SELL, 150.5, 30));   // id 1
    EXPECT_EQ(trades, 0);                                        // <- wide

    // ── step 2 : crossing sell 148.8 x40  → fully fills against bid ──────
    auto s2 = makeLimit("AAPL", OrderSide::SELL, 148.8, 40);     // id 2
    eng.submit(s2);

    EXPECT_EQ(trades, 1);            // one trade event
    EXPECT_FALSE(s2->isActive());    // sell fully filled
    EXPECT_EQ(s2->getQuantity(), 0);
    ASSERT_NE(eng.getBook("AAPL")->getBuyOrders().front(), nullptr);
    EXPECT_EQ(eng.getBook("AAPL")->getBuyOrders().front()->getQuantity(), 10);

    // ── step 3 : market buy 100  → sweeps remaining ask 150.5 x30 ────────
    eng.submit(makeMkt("AAPL", OrderSide::BUY, 100));
    EXPECT_EQ(trades, 2);            // second trade event

    // remaining book sanity
    auto ask = eng.getBook("AAPL")->getBestAsk();
    EXPECT_EQ(ask, nullptr);         // sell side empty
}


// -----------------------------------------------------------------------------
// Suite 8 : stress – 1000 orders and cancellations
// -----------------------------------------------------------------------------
TEST(EngineStress, ManyOrders)
{
    ExecutionEngine eng;
    constexpr int N = 1000;
    for (int i = 0; i < N; ++i)
        eng.submit(makeLimit("AAPL", OrderSide::BUY, 100.0 + i*0.01, 1));

    EXPECT_EQ(eng.getBook("AAPL")->getBuyOrders().size(), N);
    for (int id = 0; id < N; id += 2)
        eng.cancel(id);

    EXPECT_GE(eng.getBook("AAPL")->getBuyOrders().size(), N/2);
}
