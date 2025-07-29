#include "OrderBook.hpp"
#include <algorithm>

namespace {
constexpr double BUY_MKT_KEY  =  std::numeric_limits<double>::max();
constexpr double SELL_MKT_KEY = -std::numeric_limits<double>::max();

inline double keyFor(const std::shared_ptr<Order>& o)
{
    if (o->getType() == OrderType::MARKET)
        return (o->getSide() == OrderSide::BUY) ? BUY_MKT_KEY : SELL_MKT_KEY;
    return o->getPrice();
}
}

OrderBook::OrderBook(const std::string &symbol) : symbol(symbol) {}

int OrderBook::addOrder(const std::shared_ptr<Order> &order) {
    if (!order || order->getSymbol() != symbol) {
        throw std::invalid_argument("Order symbol mismatch or null order");
    }

    std::lock_guard lock(mtx);
    int id = order->getOrderId();
    ordersById[id] = order;
    insertOrder(order);
    return id;
}

bool OrderBook::removeOrder(int orderId) {
    std::lock_guard lock(mtx);
    auto it = ordersById.find(orderId);
    if (it == ordersById.end()) return false;

    it->second->cancel();
    eraseOrder(it->second);
    ordersById.erase(it);
    return true;
}

bool OrderBook::modifyOrder(int orderId, std::optional<double> newPrice, std::optional<int> newQty) {
    std::lock_guard lock(mtx);
    auto it = ordersById.find(orderId);

    if (it == ordersById.end()) return false;
    auto &ord = it->second;
    if (!ord->isActive()) return false;

    eraseOrder(ord);
    double price = newPrice.value_or(ord->getPrice());
    int qty = newQty.value_or(ord->getQuantity());

    if (qty <= 0) {
        ord->cancel();
        ordersById.erase(it);
        return true;
    }

    ord->modify(price, qty);
    insertOrder(ord);
    return true;
}

std::shared_ptr<Order> OrderBook::getBestBid() const
{
    std::lock_guard lock(mtx);
    for (const auto& [price, q] : buyOrders)
        for (const auto& o : q)
            if (o->isActive() && o->getType() != OrderType::MARKET)
                return o;
    return nullptr;
}

std::shared_ptr<Order> OrderBook::getBestAsk() const
{
    std::lock_guard lock(mtx);
    for (const auto& [price, q] : sellOrders)
        for (const auto& o : q)
            if (o->isActive() && o->getType() != OrderType::MARKET)
                return o;
    return nullptr;
}

std::vector<std::shared_ptr<Order>> OrderBook::getBuyOrders() const {
    std::lock_guard lock(mtx);
    std::vector<std::shared_ptr<Order>> out;
    for (const auto &[price, queue] : buyOrders) {
        for (const auto &o : queue)
            if (o->isActive()) out.push_back(o);
    }
    return out;
}

std::vector<std::shared_ptr<Order>> OrderBook::getSellOrders() const {
    std::lock_guard lock(mtx);
    std::vector<std::shared_ptr<Order>> out;
    for (const auto &[price, queue] : sellOrders) {
        for (const auto &o : queue)
            if (o->isActive()) out.push_back(o);
    }
    return out;
}

std::vector<Match> OrderBook::match() {
    std::lock_guard lock(mtx);
    std::vector<Match> executions;

    while (!buyOrders.empty() && !sellOrders.empty()) {
        auto &buyQ = buyOrders.begin()->second;
        auto &sellQ = sellOrders.begin()->second;

        while (!buyQ.empty() && !buyQ.front()->isActive()) buyQ.pop_front();
        while (!sellQ.empty() && !sellQ.front()->isActive()) sellQ.pop_front();

        if (buyQ.empty()) { 
            buyOrders.erase(buyOrders.begin()); 
            continue;
        }

        if (sellQ.empty()) {
            sellOrders.erase(sellOrders.begin());
            continue;
        }

        auto buy = buyQ.front();
        auto sell = sellQ.front();

        bool crossed = buy->getType() == OrderType::MARKET || sell->getType() == OrderType::MARKET || buy->getPrice() >= sell->getPrice();
        if (!crossed) break;

        int qty = std::min(buy->getQuantity(), sell->getQuantity());
        double px;
        if      (buy ->getType() == OrderType::MARKET) px = sell->getPrice();
        else if (sell->getType() == OrderType::MARKET) px = buy ->getPrice();
        else                                           
            px = (buy->getPrice() >= sell->getPrice())
                ? buy ->getPrice()   // buy resting -> trade @ bid 
                : sell->getPrice();  // sell resting -> trade @ ask 


        buy->reduceQuantity(qty);
        sell->reduceQuantity(qty);

        executions.push_back({buy->getOrderId(), sell->getOrderId(), px, qty});

        if (!buy->isActive()) {
            buyQ.pop_front();
            ordersById.erase(buy->getOrderId());
            if (buyQ.empty()) buyOrders.erase(buyOrders.begin());
        }

        if (!sell->isActive()) {
            sellQ.pop_front();
            ordersById.erase(sell->getOrderId());
            if (sellQ.empty()) sellOrders.erase(sellOrders.begin());
        }
    }

    return executions;
}

const std::string &OrderBook::getSymbol() const { return symbol; }

void OrderBook::insertOrder(const std::shared_ptr<Order>& o)
{
    double k = keyFor(o);
    if (o->getSide() == OrderSide::BUY)
        buyOrders[k].push_back(o);
    else
        sellOrders[k].push_back(o);
}

void OrderBook::eraseOrder(const std::shared_ptr<Order>& o)
{
    double k = keyFor(o);

    if (o->getSide() == OrderSide::BUY) {
        auto it = buyOrders.find(k);
        if (it == buyOrders.end()) return;
        auto& q = it->second;
        q.erase(std::remove(q.begin(), q.end(), o), q.end());
        if (q.empty()) buyOrders.erase(it);

    } else {
        auto it = sellOrders.find(k);
        if (it == sellOrders.end()) return;
        auto& q = it->second;
        q.erase(std::remove(q.begin(), q.end(), o), q.end());
        if (q.empty()) sellOrders.erase(it);
    }
}


std::shared_ptr<Order> OrderBook::getOrder(int orderId) const
{
    std::lock_guard lock(mtx);
    auto it = ordersById.find(orderId);
    return (it == ordersById.end()) ? nullptr : it->second;
}




