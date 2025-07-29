#pragma once

#include <unordered_map>
#include <memory>
#include <functional>
#include <string>
#include <vector>
#include <mutex>
#include "OrderBook.hpp"

class ExecutionEngine {
public:

    struct Trade {
        std::string symbol;
        int buyId;
        int sellId;
        double price;
        int qty;
    };

    using TradeHandler = std::function<void(const Trade&)>;
    explicit ExecutionEngine();

    void ensureBook(const std::string& symbol);
    OrderBook* getBook(const std::string& symbol);
    const OrderBook* getBook(const std::string &symbol) const;

    int submit(const std::shared_ptr<Order>& order);
    bool cancel(int orderId);
    bool modify(int orderId, 
        std::optional<double> newPrice = std::nullopt, 
        std::optional<int> newQty = std::nullopt);

    void setMaxOrderQty(int maxQty) { maxOrderQty_ = maxQty; }
    void setTradeHandler(TradeHandler cb) { tradeCb_ = std::move(cb); }

private:
    OrderBook* bookForOrder(int orderId);
    std::unordered_map<std::string, std::unique_ptr<OrderBook>> books_;
    std::unordered_map<int, OrderBook*> idToBook_;
    mutable std::mutex booksMtx_;
    int maxOrderQty_ {1'000'000};
    TradeHandler tradeCb_ {nullptr};
};

