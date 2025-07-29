#pragma once 

#include <string>
#include <atomic>
#include <stdexcept>

enum class OrderSide { BUY, SELL };
enum class OrderType { LIMIT, MARKET, STOP };

class Order{
public:
    Order(const std::string &symbol, OrderSide side, OrderType type, double price, int quantity);

    Order(const Order &) = default;
    Order(Order &&) noexcept = default;
    Order &operator=(const Order &) = default;
    Order &operator=(Order &&) noexcept = default;
    ~Order() = default;

    int getOrderId() const;
    OrderSide getSide() const;
    OrderType getType() const;
    double getPrice() const;
    int getQuantity() const;
    bool isActive() const;
    const std::string &getSymbol() const;

    void modify(double newPrice, int newQuantity);
    void reduceQuantity(int tradedQty);

    void cancel();

private:
    static std::atomic<int> nextOrderId;
    int orderId;
    std::string symbol;
    OrderSide side;
    OrderType type;
    double price;
    int quantity;
    bool active;
};