#pragma once

#include <vector>
#include <map>
#include <deque>
#include <memory>
#include <optional>
#include <string>
#include <mutex>
#include <limits>
#include <functional>
#include "Order.hpp"

struct Match {
    int    buyId;
    int    sellId;
    double price;
    int    qty;
};


class OrderBook {
public:
    explicit OrderBook(const std::string &symbol);
    int addOrder(const std::shared_ptr<Order> &order);
    bool removeOrder(int orderId);
    bool modifyOrder(int orderId, std::optional<double> newPrice = std::nullopt, std::optional<int> newQty = std::nullopt);

    std::shared_ptr<Order> getBestBid() const;
    std::shared_ptr<Order> getBestAsk() const;
    std::shared_ptr<Order> getOrder(int orderId) const;
    std::vector<std::shared_ptr<Order>> getBuyOrders() const;
    std::vector<std::shared_ptr<Order>> getSellOrders() const;

    std::vector<Match> match();
    const std::string &getSymbol() const;

private:
    static constexpr double BUY_MKT_KEY  =  std::numeric_limits<double>::max();
    static constexpr double SELL_MKT_KEY = -std::numeric_limits<double>::max();
    void insertOrder(const std::shared_ptr<Order> &o);
    void eraseOrder(const std::shared_ptr<Order> &o);

    std::string symbol;
    std::map<int, std::shared_ptr<Order>> ordersById;

    std::map<double, std::deque<std::shared_ptr<Order>>, std::greater<>> buyOrders;
    std::map<double, std::deque<std::shared_ptr<Order>>, std::less<>> sellOrders;
    mutable std::mutex mtx;
};

