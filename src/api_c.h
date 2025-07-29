#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct { double px; int qty; } tcx_level;

typedef void* tcx_engine;
typedef void* tcx_order;

enum tcx_side { TCX_BUY  = 0, TCX_SELL  = 1 };
enum tcx_type { TCX_LIMIT= 1, TCX_MARKET= 2, TCX_STOP = 3 };

tcx_engine tcx_create_engine(void);
void       tcx_destroy_engine(tcx_engine e);

tcx_order  tcx_order_new(const char* symbol,
                         enum tcx_side side,
                         enum tcx_type type,
                         double price,
                         int    qty);
void       tcx_order_free(tcx_order o);

int  tcx_submit(tcx_engine e, tcx_order o);  
int  tcx_cancel(tcx_engine e, int orderId);  
int  tcx_modify(tcx_engine e, int orderId,
                double newPx /*≤0 keep*/,
                int    newQty/*≤0 keep*/);

void tcx_poll(tcx_engine e);

enum tcx_evt_type { TCX_EVT_TRADE=0, TCX_EVT_TOB=1 };

struct tcx_evt {
    enum tcx_evt_type type;
    char   symbol[16];
    int    buyId;
    int    sellId;
    int    qty;
    double px;

    double bidPx;
    int    bidQty;
    double askPx;
    int    askQty;
};

int tcx_next_event(tcx_engine eng, struct tcx_evt* out);

int tcx_depth(tcx_engine         h,
              const char*        symbol,
              int                levels,
              tcx_level*         bidBuf,
              int*               nBids,
              tcx_level*         askBuf,
              int*               nAsks);


#ifdef __cplusplus
}   /* extern "C" */
#endif



