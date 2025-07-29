#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include "EngineRunner.hpp"

using namespace std::chrono_literals;

void printEvent(const OutboundMsg& evt)
{
    std::visit([](auto&& e)
    {
        using T = std::decay_t<decltype(e)>;
        if constexpr (std::is_same_v<T, TradeEvent>) {
            std::cout << "[TRADE] " << e.fill.symbol
                      << "  buyId="  << e.fill.buyId
                      << "  sellId=" << e.fill.sellId << '\n';

        } else if constexpr (std::is_same_v<T, TopOfBookEvt>) {
            std::cout << std::fixed << std::setprecision(2)
                      << "[TOB]  " << e.symbol << "  "
                      << e.bidQty << '@' << e.bidPx << "  /  "
                      << e.askQty << '@' << e.askPx << '\n';
        }
    }, evt);
}

int main()
{
    EngineRunner runner;

    runner.push(NewOrderMsg{ std::make_shared<Order>("AAPL", OrderSide::BUY,
                              OrderType::LIMIT, 179.50, 50) });
    runner.push(NewOrderMsg{ std::make_shared<Order>("AAPL", OrderSide::BUY,
                              OrderType::LIMIT, 179.80, 100) });
    runner.push(NewOrderMsg{ std::make_shared<Order>("AAPL", OrderSide::SELL,
                              OrderType::LIMIT, 180.10, 60) });
    runner.push(NewOrderMsg{ std::make_shared<Order>("AAPL", OrderSide::SELL,
                              OrderType::LIMIT, 180.40, 40) });

    std::this_thread::sleep_for(5ms);

    OutboundMsg ev;
    while (runner.poll(ev)) printEvent(ev);

    runner.push(NewOrderMsg{ std::make_shared<Order>("AAPL", OrderSide::BUY,
                              OrderType::MARKET, 0.0, 60) });
    std::this_thread::sleep_for(5ms);
    while (runner.poll(ev)) printEvent(ev);

    runner.push(ModifyMsg{0 /*id*/, 179.90, std::nullopt});
    std::this_thread::sleep_for(5ms);
    while (runner.poll(ev)) printEvent(ev);

    runner.push(NewOrderMsg{ std::make_shared<Order>("AAPL", OrderSide::SELL,
                              OrderType::MARKET, 0.0, 140) });
    std::this_thread::sleep_for(5ms);
    while (runner.poll(ev)) printEvent(ev);

    runner.push(CancelMsg{3});
    std::this_thread::sleep_for(5ms);
    while (runner.poll(ev)) printEvent(ev);

    runner.push(NewOrderMsg{ std::make_shared<Order>("AAPL", OrderSide::BUY,
                              OrderType::LIMIT, 180.50, 20) });
    std::this_thread::sleep_for(5ms);
    while (runner.poll(ev)) printEvent(ev);

    std::cout << "\nSimulation finished — shutting down.\n";
    runner.stop();
}
