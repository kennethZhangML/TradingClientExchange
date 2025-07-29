#include "EngineRunner.hpp"

EngineRunner::EngineRunner()
{
    eng_.setTradeHandler([this](const ExecutionEngine::Trade& t){
        std::lock_guard lk(mtx_);
        outQ_.push(TradeEvent{t});
    });
    worker_ = std::thread([this]{ loop(); });
}

EngineRunner::~EngineRunner()
{
    stop();
    if (worker_.joinable()) worker_.join();
}

void EngineRunner::push(const InboundMsg& m) {
    { std::lock_guard lk(mtx_); inQ_.push(m); }
    cv_.notify_one();
}

bool EngineRunner::poll(OutboundMsg& out)
{
    std::lock_guard lk(mtx_);
    if (outQ_.empty()) return false;
    out = std::move(outQ_.front());
    outQ_.pop();
    return true;
}

void EngineRunner::stop()
{
    running_.store(false);
    cv_.notify_one();
}

void EngineRunner::loop()
{
    static const std::string emptySym;
    while (running_.load())
    {
        InboundMsg msg;
        {
            std::unique_lock lk(mtx_);
            cv_.wait(lk, [&]{ return !inQ_.empty() || !running_.load(); });
            if (!running_.load()) break;
            msg = std::move(inQ_.front());
            inQ_.pop();
        }

        const std::string* symPtr = &emptySym;

        std::visit([&](auto&& m){
            using T = std::decay_t<decltype(m)>;

            if constexpr (std::is_same_v<T, NewOrderMsg>) {
                symPtr = &m.order->getSymbol();
                eng_.submit(m.order);

            } else if constexpr (std::is_same_v<T, CancelMsg>) {
                eng_.cancel(m.orderId);

            } else if constexpr (std::is_same_v<T, ModifyMsg>) {
                eng_.modify(m.orderId, m.px, m.qty);
            }
        }, msg);

        if (!symPtr->empty()) {
            if (auto* book = eng_.getBook(*symPtr)) {
                auto bid = book->getBestBid();
                auto ask = book->getBestAsk();

                std::lock_guard lk(mtx_);
                outQ_.push(TopOfBookEvt{
                    *symPtr,
                    bid ? bid->getPrice()    : 0.0,
                    bid ? bid->getQuantity() : 0,
                    ask ? ask->getPrice()    : 0.0,
                    ask ? ask->getQuantity() : 0
                });
            }
        }
    }
}
