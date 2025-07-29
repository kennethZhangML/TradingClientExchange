#pragma once 

#include <variant>
#include <queue>
#include <thread>
#include <condition_variable>
#include <atomic>

#include "ExecutionEngine.hpp"

struct NewOrderMsg { std::shared_ptr<Order> order; };
struct CancelMsg { int orderId; };
struct ModifyMsg { int orderId; std::optional<double> px; std::optional<int> qty; };
using InboundMsg = std::variant<NewOrderMsg, CancelMsg, ModifyMsg>;

struct TradeEvent { ExecutionEngine::Trade fill; };
struct TopOfBookEvt { std::string symbol; double bidPx; int bidQty; double askPx; int askQty; };
using OutboundMsg = std::variant<TradeEvent, TopOfBookEvt>;

class EngineRunner { 
public:
    EngineRunner();
    ~EngineRunner();

    void push(const InboundMsg& msg);
    bool poll(OutboundMsg& out);
    void stop();

    ExecutionEngine& engine() { return eng_; }
    const ExecutionEngine& engine() const { return eng_; }

private:
    void loop();
    
    ExecutionEngine eng_;
    std::thread worker_;

    std::queue<InboundMsg> inQ_;
    std::queue<OutboundMsg> outQ_;

    std::mutex mtx_;
    std::condition_variable cv_;
    std::atomic<bool> running_{true};
};