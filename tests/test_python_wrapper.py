import sys, pathlib, time
ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "research" / "py"))

from tcx import Engine, BUY, SELL, Trade

def wait_for(predicate, timeout=0.2, sleep=0.001):
    """Spin until predicate() returns truthy or timeout (returns value)."""
    deadline = time.time() + timeout
    while time.time() < deadline:
        val = predicate()
        if val:
            return val
        time.sleep(sleep)
    return None

def test_limit_match_and_depth_cleanup():
    with Engine() as eng:
        bid_id = eng.submit_limit("TEST", BUY , 10.0, 5)
        ask_id = eng.submit_limit("TEST", SELL, 10.0, 5)

        def done():
            events = eng.poll()
            bids, asks = eng.depth("TEST")
            return (events, bids, asks)

        events, bids, asks = wait_for(lambda: done() if not any(done()[:2]) else done())

        assert bids == [] and asks == []

        trades = [e for e in events if isinstance(e, Trade)]
        if trades:
            t = trades[-1]
            assert {t.buy_id, t.sell_id} == {bid_id, ask_id}
            assert t.qty == 5 and t.px == 10.0


def test_modify_and_cancel():
    with Engine() as eng:
        oid = eng.submit_limit("IBM", BUY, 100.0, 20)
        eng.poll()                               # flush submit

        eng.modify(oid, px=101.0, qty=10)
        eng.poll()                               # flush modify
        bids, _ = eng.depth("IBM")
        assert bids == [(101.0, 10)]

        eng.cancel(oid)
        eng.poll()                               # flush cancel
        bids, _ = eng.depth("IBM")
        assert bids == []


def test_market_order_hits_best():
    with Engine() as eng:
        _ = eng.submit_limit("MSFT", SELL, 310.0, 7)
        eng.poll()                               # flush submit

        _ = eng.submit_market("MSFT", BUY, 7)

        def done():
            events = eng.poll()
            _, asks = eng.depth("MSFT")
            return (events, asks)

        events, asks = wait_for(lambda: done() if done()[1] else done())

        assert asks == [], "Ask book not empty after market buy"

        trades = [e for e in events if isinstance(e, Trade)]
        if trades:
            t = trades[-1]
            assert t.px == 310.0 and t.qty == 7