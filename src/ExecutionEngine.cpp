#include "ExecutionEngine.hpp"
#include <stdexcept>

ExecutionEngine::ExecutionEngine() = default;

void ExecutionEngine::ensureBook(const std::string& symbol) {
    std::lock_guard lock(booksMtx_);
    if (books_.find(symbol) == books_.end())
        books_.emplace(symbol, std::make_unique<OrderBook>(symbol));
}

OrderBook* ExecutionEngine::getBook(const std::string& symbol) {
    std::lock_guard lock(booksMtx_);
    auto it = books_.find(symbol);
    return (it == books_.end()) ? nullptr : it->second.get();
}

const OrderBook* ExecutionEngine::getBook(const std::string& symbol) const {
    std::lock_guard lock(booksMtx_);
    auto it = books_.find(symbol);
    return (it == books_.end()) ? nullptr : it->second.get();
}

int ExecutionEngine::submit(const std::shared_ptr<Order>& o)
{
    if(!o)                           throw std::invalid_argument("null order");
    if(o->getQuantity()>maxOrderQty_)throw std::invalid_argument("qty too big");

    ensureBook(o->getSymbol());
    auto* book = getBook(o->getSymbol());
    int id = book->addOrder(o);

    { std::lock_guard lk(booksMtx_); idToBook_[id]=book; }

    auto fills = book->match();
    if(tradeCb_ && !fills.empty()){
        for(const auto& m: fills)
            tradeCb_({o->getSymbol(), m.buyId, m.sellId, m.price, m.qty});
    }
    return id;
}

bool ExecutionEngine::cancel(int id)
{
    OrderBook* book;
    { std::lock_guard lk(booksMtx_);
      auto it=idToBook_.find(id); if(it==idToBook_.end()) return false;
      book=it->second; }

    bool ok = book->removeOrder(id);
    if(ok){ std::lock_guard lk(booksMtx_); idToBook_.erase(id); }
    return ok;
}

bool ExecutionEngine::modify(int id,
                             std::optional<double> px,
                             std::optional<int> qt)
{
    auto* book = bookForOrder(id);
    if(!book) return false;

    if(!book->modifyOrder(id, px, qt)) return false;

    auto fills = book->match();
    if(tradeCb_ && !fills.empty()){
        const auto& sym = book->getSymbol();
        for(const auto& m: fills)
            tradeCb_({sym, m.buyId, m.sellId, m.price, m.qty});
    }
    return true;
}

OrderBook* ExecutionEngine::bookForOrder(int orderId) {
    std::lock_guard lock(booksMtx_);
    auto it = idToBook_.find(orderId);
    return (it == idToBook_.end()) ? nullptr : it->second;
}




