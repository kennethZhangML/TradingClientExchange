from dataclasses import dataclass
from enum import Enum, auto
import sys

def _dcls(_cls=None, *, slots=True, **kw):
    """dataclass decorator that ignores slots on <3.10"""
    def wrap(cls):
        if slots and sys.version_info < (3, 10):
            return dataclass(**kw)(cls)        # drop slots
        return dataclass(slots=slots, **kw)(cls)
    return wrap if _cls is None else wrap(_cls)

class EventType(Enum):
    TRADE = auto()
    TOB   = auto()

@_dcls
class Trade:
    symbol:  str
    buy_id:  int
    sell_id: int
    qty:     int
    px:      float
    type:    EventType = EventType.TRADE

@_dcls
class TopOfBook:
    symbol:  str
    bid_px:  float
    bid_qty: int
    ask_px:  float
    ask_qty: int
    type:    EventType = EventType.TOB
