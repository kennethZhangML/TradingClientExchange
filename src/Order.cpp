#include "Order.hpp"

std::atomic<int> Order::nextOrderId{0};

Order::Order(const std::string &symbol, OrderSide side, OrderType type, double price, int quantity) : 
    symbol(symbol),
    side(side),
    type(type),
    price(price),
    quantity(quantity),
    active(true) 
{
    if (symbol.empty()) throw std::invalid_argument("Symbol must not be empty");
    if (quantity <= 0) throw std::invalid_argument("Quantity must be positive");
    if (type != OrderType::MARKET && price <= 0.0) {
        throw std::invalid_argument("Price must be positive for non-market orders");
    }
    orderId = nextOrderId.fetch_add(1, std::memory_order_relaxed);
}


int Order::getOrderId() const { return orderId; }
OrderSide Order::getSide() const { return side; }
OrderType Order::getType() const { return type; }
double Order::getPrice() const { return price; }
int Order::getQuantity() const { return quantity; }
bool Order::isActive() const { return active; }
const std::string &Order::getSymbol() const { return symbol; }

void Order::modify(double newPrice, int newQuantity) {
    if (!active) throw std::logic_error("Cannot modify a cancelled/filled order");
    if (newQuantity < 0) throw std::invalid_argument("Quantity cannot be negative");

    if (type != OrderType::MARKET && newPrice <= 0.0) {
        throw std::invalid_argument("Price must positive for non-market orders");
    }

    price = newPrice;
    quantity = newQuantity;

    if (quantity == 0) active = false;
}

void Order::reduceQuantity(int tradedQty) {
    if (tradedQty <= 0) throw std::invalid_argument("Traded qty must be positive");
    if (tradedQty > quantity) throw std::invalid_argument("Traded qty exceeds resting qty");
    modify(price, quantity - tradedQty);
}

void Order::cancel() {
    active = false;
    quantity = 0;
}




