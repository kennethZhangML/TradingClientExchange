"""
Thin Python wrapper over the C² engine (api_c.cpp).

Only the functionality needed by research scripts is exposed:
    • submit_limit / submit_market
    • cancel / modify
    • poll()  – list of Trade / TopOfBook objects
    • depth() – N levels of bids / asks
"""

from __future__ import annotations
import ctypes, pathlib, sys
from enum import IntEnum
from typing import List, Tuple
import os 

_root = pathlib.Path(__file__).resolve().parents[3]      # project root
_dll  = None

def _find_lib() -> pathlib.Path:
    env = os.environ.get("TCX_LIB")          # 1) explicit override
    if env and pathlib.Path(env).exists():
        return pathlib.Path(env)

    search_dirs = [
        _root / "build",
        *(_root / "build").glob("*/"),       # e.g. build/Debug/
    ]
    for d in search_dirs:
        for ext in (".dylib", ".so"):
            cand = list(d.glob(f"libtce_core*{ext}"))
            if cand:
                return cand[0]
    raise FileNotFoundError(
        "libtce_core shared library not found. "
        "Re‑build with `cmake .. -DBUILD_SHARED_LIBS=ON && make -j`, "
        "or set TCX_LIB to its full path."
    )

_dll = _find_lib()
lib = ctypes.CDLL(str(_dll))

class Side  (IntEnum): BUY = 0; SELL = 1
class OrdType(IntEnum): LIMIT = 1; MARKET = 2; STOP = 3
BUY, SELL   = Side.BUY, Side.SELL
LIMIT, MARKET, STOP = (OrdType.LIMIT, OrdType.MARKET, OrdType.STOP)

class EventType(IntEnum): TRADE = 1; TOB = 2

class _Depth(ctypes.Structure):
    _fields_ = [("px",  ctypes.c_double),
                ("qty", ctypes.c_int)]

class _Evt(ctypes.Structure):
    _fields_ = [("type",    ctypes.c_int),
                ("symbol",  ctypes.c_char * 16),
                ("buyId",   ctypes.c_int),
                ("sellId",  ctypes.c_int),
                ("qty",     ctypes.c_int),
                ("px",      ctypes.c_double),
                ("bidPx",   ctypes.c_double),
                ("bidQty",  ctypes.c_int),
                ("askPx",   ctypes.c_double),
                ("askQty",  ctypes.c_int)]

lib.tcx_create_engine.restype = ctypes.c_void_p
lib.tcx_destroy_engine.argtypes = [ctypes.c_void_p]

lib.tcx_order_new.restype = ctypes.c_void_p
lib.tcx_order_new.argtypes = [ctypes.c_char_p,
                              ctypes.c_int, ctypes.c_int,
                              ctypes.c_double, ctypes.c_int]

lib.tcx_order_free.argtypes = [ctypes.c_void_p]

lib.tcx_submit.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
lib.tcx_submit.restype  = ctypes.c_int

lib.tcx_cancel.argtypes = [ctypes.c_void_p, ctypes.c_int]
lib.tcx_modify.argtypes = [ctypes.c_void_p, ctypes.c_int,
                           ctypes.c_double, ctypes.c_int]

lib.tcx_poll.argtypes   = [ctypes.c_void_p]
lib.tcx_next_event.argtypes = [ctypes.c_void_p, ctypes.POINTER(_Evt)]
lib.tcx_next_event.restype  = ctypes.c_int

lib.tcx_depth.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_int,
                          ctypes.POINTER(_Depth), ctypes.POINTER(ctypes.c_int),
                          ctypes.POINTER(_Depth), ctypes.POINTER(ctypes.c_int)]
lib.tcx_depth.restype  = ctypes.c_int


class Trade(tuple):
    __slots__ = ()
    def __new__(cls, sym, buy_id, sell_id, qty, px):
        return super().__new__(cls, (sym, buy_id, sell_id, qty, px))
    @property
    def symbol (self): return self[0]
    @property
    def buy_id (self): return self[1]
    @property
    def sell_id(self): return self[2]
    @property
    def qty    (self): return self[3]
    @property
    def px     (self): return self[4]
    type = EventType.TRADE

class TopOfBook(tuple):
    __slots__ = ()
    def __new__(cls, sym, bp, bq, ap, aq):
        return super().__new__(cls, (sym, bp, bq, ap, aq))
    @property
    def symbol (self): return self[0]
    @property
    def bid_px (self): return self[1]
    @property
    def bid_qty(self): return self[2]
    @property
    def ask_px (self): return self[3]
    @property
    def ask_qty(self): return self[4]
    type = EventType.TOB

class Engine:
    def __init__(self):
        self._h = lib.tcx_create_engine()

    def _new_order(self, sym:str, side:Side, typ:OrdType, px:float, qty:int):
        ptr = lib.tcx_order_new(sym.encode(), side, typ, px, qty)
        oid = lib.tcx_submit(self._h, ptr)
        lib.tcx_order_free(ptr) 
        return oid

    def submit_limit (self, sym, side, px, qty):
        return self._new_order(sym, side, LIMIT , px, qty)
    def submit_market(self, sym, side, qty):
        return self._new_order(sym, side, MARKET, 0.0, qty)

    def cancel(self, order_id:int):
        lib.tcx_cancel(self._h, order_id)

    def modify(self, order_id:int, px:float=0.0, qty:int|None=0):
        lib.tcx_modify(self._h, order_id, px, 0 if qty is None else qty)

    def poll(self) -> List[Trade|TopOfBook]:
        lib.tcx_poll(self._h)     
        evt = _Evt()
        out = []
        while lib.tcx_next_event(self._h, ctypes.byref(evt)):
            if evt.type == EventType.TRADE:
                out.append(Trade(evt.symbol.decode(),
                                 evt.buyId, evt.sellId,
                                 evt.qty, evt.px))
            else:
                out.append(TopOfBook(evt.symbol.decode(),
                                     evt.bidPx, evt.bidQty,
                                     evt.askPx, evt.askQty))
        return out

    def depth(self, symbol:str, levels:int=5) -> Tuple[List[Tuple[float,int]],
                                                       List[Tuple[float,int]]]:
        BidArr = _Depth * levels
        AskArr = _Depth * levels
        bids = BidArr(); asks = AskArr()
        nB = ctypes.c_int(); nA = ctypes.c_int()
        lib.tcx_depth(self._h, symbol.encode(), levels,
                      bids, ctypes.byref(nB),
                      asks, ctypes.byref(nA))
        bid_lvls = [(bids[i].px, bids[i].qty) for i in range(nB.value)]
        ask_lvls = [(asks[i].px, asks[i].qty) for i in range(nA.value)]
        return bid_lvls, ask_lvls

    def stop(self):
        if self._h:
            lib.tcx_destroy_engine(self._h)
            self._h = None

    def __enter__(self):  return self
    def __exit__(self, *exc): self.stop()

__all__ = ["Engine", "BUY", "SELL", "LIMIT", "MARKET", "STOP",
           "Trade", "TopOfBook"]
