#include "api_c.h"
#include "EngineRunner.hpp"
#include <memory>
#include <mutex>
#include <vector>
#include <cstdio>

struct CEngine {
    EngineRunner runner;
    std::vector<OutboundMsg> buf;
};

struct DepthLevel { double px; int qty; };

tcx_engine tcx_create_engine() { return new CEngine();  }
void       tcx_destroy_engine(tcx_engine h){ delete (CEngine*)h; }

static std::shared_ptr<Order> makeShared(const char* s,
                                         tcx_side sd, tcx_type tp,
                                         double px, int qty)
{
    return std::make_shared<Order>(
        s,
        sd==TCX_BUY ? OrderSide::BUY : OrderSide::SELL,
        tp==TCX_LIMIT ? OrderType::LIMIT :
        tp==TCX_MARKET? OrderType::MARKET : OrderType::STOP,
        px, qty);
}
tcx_order tcx_order_new(const char* sym,
                        tcx_side sd, tcx_type tp,
                        double px, int qty)
{
    return new std::shared_ptr<Order>(makeShared(sym,sd,tp,px,qty));
}
void tcx_order_free(tcx_order p) { delete (std::shared_ptr<Order>*)p; }

int tcx_submit(tcx_engine h, tcx_order o)
{
    auto  eng  = (CEngine*)h;
    auto* sptr = (std::shared_ptr<Order>*)o;
    int   id   = (*sptr)->getOrderId();
    eng->runner.push(NewOrderMsg{*sptr});
    return id;
}
int tcx_cancel(tcx_engine h,int id)
{
    ((CEngine*)h)->runner.push(CancelMsg{id});
    return 0;
}
int tcx_modify(tcx_engine h,int id,double px,int qty)
{
    std::optional<double> npx = (px  >0) ? std::optional<double>(px)  : std::nullopt;
    std::optional<int>    nqt = (qty >0) ? std::optional<int>(qty)    : std::nullopt;
    ((CEngine*)h)->runner.push(ModifyMsg{id,npx,nqt});
    return 0;
}

static void printEvent(const OutboundMsg& ev)
{
    std::visit([](auto&& e){
        using T = std::decay_t<decltype(e)>;
        if constexpr (std::is_same_v<T, TradeEvent>)
            std::printf("FILL  %s  buy=%d  sell=%d\n",
                        e.fill.symbol.c_str(),
                        e.fill.buyId, e.fill.sellId);
        else
            std::printf("TOB   %s  %d@%.2f  /  %d@%.2f\n",
                        e.symbol.c_str(),
                        e.bidQty, e.bidPx,
                        e.askQty, e.askPx);
    }, ev);
}

static bool to_c_evt(const OutboundMsg& ev, tcx_evt& out)
{
    memset(&out, 0, sizeof(out));
    return std::visit([&](auto&& e)->bool{
        using T = std::decay_t<decltype(e)>;
        if constexpr (std::is_same_v<T,TradeEvent>){
            out.type   = TCX_EVT_TRADE;
            std::strncpy(out.symbol, e.fill.symbol.c_str(), 15);
            out.buyId  = e.fill.buyId;
            out.sellId = e.fill.sellId;
            out.qty    = e.fill.qty;
            out.px     = e.fill.price;
        } else {
            out.type    = TCX_EVT_TOB;
            std::strncpy(out.symbol, e.symbol.c_str(), 15);
            out.bidPx   = e.bidPx;
            out.bidQty  = e.bidQty;
            out.askPx   = e.askPx;
            out.askQty  = e.askQty;
        }
        return true;
    }, ev);
}


void tcx_poll(tcx_engine h)
{
    auto* eng = (CEngine*)h;
    OutboundMsg ev;
    while (eng->runner.poll(ev))
        eng->buf.push_back(ev); 
}

int tcx_next_event(tcx_engine h, tcx_evt* out)
{
    auto* eng = (CEngine*)h;
    if (eng->buf.empty()) return 0;
    to_c_evt(eng->buf.front(), *out);
    eng->buf.erase(eng->buf.begin());
    return 1;
}

extern "C"
int tcx_depth(tcx_engine         h,
              const char*        symbol,
              int                levels,
              tcx_level*         bidBuf,
              int*               nBids,
              tcx_level*         askBuf,
              int*               nAsks)
{
    auto* book = ((CEngine*)h)->runner.engine().getBook(symbol);
    if (!book) { *nBids = *nAsks = 0; return 0; }

    auto bids = book->getBuyOrders();
    auto asks = book->getSellOrders();

    int nb = std::min<int>(levels, bids.size());
    int na = std::min<int>(levels, asks.size());

    for (int i = 0; i < nb; ++i) {
        bidBuf[i].px  = bids[i]->getPrice();
        bidBuf[i].qty = bids[i]->getQuantity();
    }
    for (int i = 0; i < na; ++i) {
        askBuf[i].px  = asks[i]->getPrice();
        askBuf[i].qty = asks[i]->getQuantity();
    }
    *nBids = nb;
    *nAsks = na;
    return nb + na;
}



