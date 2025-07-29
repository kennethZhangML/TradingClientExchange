"""
research/py/tcx/_ffi.py
ctypes bindings to libtce_core  (safe version)
"""

import ctypes, pathlib
from ctypes.util import find_library
from ctypes import (c_void_p, c_char_p, c_int,
                    c_double, Structure)

# ── locate shared library ------------------------------------------------
_repo = pathlib.Path(__file__).resolve().parents[3]       # project root
_build = _repo / 'build'
_candidates = list(_build.glob('libtce_core.*')) if _build.exists() else []

if _candidates:
    _dll_path = _candidates[0]
else:
    _name = find_library('tce_core')
    if not _name:
        raise FileNotFoundError(
            "libtce_core not found. Build with BUILD_SHARED_LIBS=ON "
            "or add it to DYLD_LIBRARY_PATH / LD_LIBRARY_PATH.")
    _dll_path = _name

lib = ctypes.CDLL(str(_dll_path))

class _Evt(ctypes.Structure):
    _fields_=[('type',   ctypes.c_int),
              ('symbol', ctypes.c_char*16),
              ('buyId',  ctypes.c_int),
              ('sellId', ctypes.c_int),
              ('qty',    ctypes.c_int),
              ('px',     ctypes.c_double),
              ('bidPx',  ctypes.c_double),
              ('bidQty', ctypes.c_int),
              ('askPx',  ctypes.c_double),
              ('askQty', ctypes.c_int)]
lib.tcx_poll.argtypes        = (c_void_p,)          # already set
lib.tcx_next_event.argtypes  = (c_void_p, ctypes.POINTER(_Evt))
lib.tcx_next_event.restype   = c_int


# ── mirror C enums -------------------------------------------------------
BUY, SELL = 0, 1
LIMIT, MARKET, STOP = 1, 2, 3

# ── opaque handles -------------------------------------------------------
lib.tcx_create_engine.restype = c_void_p
lib.tcx_destroy_engine.argtypes = (c_void_p,)

lib.tcx_order_new.restype = c_void_p
lib.tcx_order_new.argtypes = (c_char_p, c_int, c_int, c_double, c_int)
lib.tcx_order_free.argtypes = (c_void_p,)

# submit / cancel / modify
lib.tcx_submit.argtypes = (c_void_p, c_void_p)
lib.tcx_submit.restype  = c_int

lib.tcx_cancel.argtypes = (c_void_p, c_int)
lib.tcx_modify.argtypes = (c_void_p, c_int, c_double, c_int)

# poll (‼ MUST have argtypes or we crash ‼)
lib.tcx_poll.argtypes = (c_void_p,)
lib.tcx_poll.restype  = None

# depth
class _Level(Structure):
    _fields_ = [("px",  c_double),
                ("qty", c_int)]

lib.tcx_depth.argtypes = (c_void_p, c_char_p,
                          ctypes.POINTER(_Level), c_int,
                          ctypes.POINTER(_Level), c_int)
lib.tcx_depth.restype  = c_int
