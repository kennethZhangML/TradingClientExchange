#include <gtest/gtest.h>
#include <thread>
#include <atomic>
#include "EngineRunner.hpp"

using std::make_shared;

static auto mktBuy (int q){return make_shared<Order>("AAPL",OrderSide::BUY ,
                                                    OrderType::MARKET,0.0,q);}
static auto mktSell(int q){return make_shared<Order>("AAPL",OrderSide::SELL,
                                                    OrderType::MARKET,0.0,q);}
static auto lim  (double px,int q,OrderSide s){
    return make_shared<Order>("AAPL",s,OrderType::LIMIT,px,q);
}


TEST(EngineRunnerBasic, PushAndPollOrderFlow)
{
    EngineRunner r;

    r.push(NewOrderMsg{ lim(150.0, 50, OrderSide::BUY ) }); // id 0
    r.push(NewOrderMsg{ lim(149.5, 25, OrderSide::SELL) }); // id 1
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    int tobCnt = 0, tradeCnt = 0;
    OutboundMsg ev;
    while (r.poll(ev))
    {
        std::visit([&](auto&& e){
            using T = std::decay_t<decltype(e)>;
            if constexpr (std::is_same_v<T, TradeEvent>)   ++tradeCnt;
            else                                          ++tobCnt;
        }, ev);
    }
    r.stop();

    EXPECT_EQ(tradeCnt, 1);
    EXPECT_GE(tobCnt, 2);
}

TEST(EngineRunnerBasic, MultiSymbolTOB)
{
    EngineRunner r;
    r.push(NewOrderMsg{ make_shared<Order>("MSFT", OrderSide::BUY ,
                                   OrderType::LIMIT, 300, 10) });
    r.push(NewOrderMsg{ make_shared<Order>("AAPL", OrderSide::SELL,
                                   OrderType::LIMIT, 180, 5 ) });

    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    bool sawAapl = false, sawMsft = false;
    OutboundMsg ev;
    while (r.poll(ev)) {
        if (auto* p = std::get_if<TopOfBookEvt>(&ev)) {
            if (p->symbol == "AAPL") sawAapl = true;
            if (p->symbol == "MSFT") sawMsft = true;
        }
    }
    r.stop();
    EXPECT_TRUE(sawAapl && sawMsft);
}

TEST(EngineRunner, TradeCallbackConvertsToOutbound)
{
    EngineRunner r;

    r.push(NewOrderMsg{ lim(150, 1, OrderSide::BUY ) });
    r.push(NewOrderMsg{ lim(149, 1, OrderSide::SELL) });

    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    int tradeEvt = 0;
    OutboundMsg ev;
    while (r.poll(ev))
        if (std::holds_alternative<TradeEvent>(ev)) ++tradeEvt;

    r.stop();
    EXPECT_EQ(tradeEvt, 1);
}

TEST(EngineRunner, StopTerminatesCleanly)
{
    {
        EngineRunner r;
        r.push(NewOrderMsg{ lim(100, 1, OrderSide::BUY) });
        // let destructor run -> should join without deadlock
    }
    SUCCEED();
}

TEST(EngineRunnerConcurrency, ManyThreads)
{
    EngineRunner r;
    constexpr int N = 10, OrdersPerThread = 100;
    std::atomic<int> done{0};

    auto worker = [&](int tid){
        for (int i=0;i<OrdersPerThread;++i)
            r.push(NewOrderMsg{ lim(100+tid, 1, OrderSide::BUY) });
        ++done;
    };

    std::vector<std::thread> th;
    for (int t=0;t<N;++t) th.emplace_back(worker,t);
    for (auto& t : th) t.join();

    while (done.load() < N)
        std::this_thread::yield();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    int tobSeen = 0;
    OutboundMsg ev;
    while (r.poll(ev))
        if (std::holds_alternative<TopOfBookEvt>(ev)) ++tobSeen;

    r.stop();
    EXPECT_GE(tobSeen, 1); 
}
