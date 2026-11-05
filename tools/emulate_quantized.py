"""Emulate the engine's quantized SendPropFloat path for DT_Beam and DT_EffectData (read-only).

Runs server.dll's own HighLowMul helper to build a SendProp exactly as the DT_Beam and
DT_EffectData registrations would, then engine.dll's own float encoder, decoder and skip reader
under Unicorn. Compares the stock props (beam 19 bits, EffectData 17 bits, +-16384) with the
expanded props (20 / 18 bits, +-32768). Addresses are for Steam build 25375506
(P4-15/16 and P5-08/09); a PE32 (x86) pair is detected and uses the X86_* addresses.
With a second ENGINE/SERVER pair (e.g. x64 and x86), both encoders must also produce identical
bitstreams for every value, and each decoder must read the other's stream like its own.

Usage: python tools/emulate_quantized.py ENGINE_DLL SERVER_DLL [ENGINE_DLL2 SERVER_DLL2]
Requires: pip install unicorn
"""
import random
import struct
import sys

from emulate_codec import BF, BUF, PROP, X86, call, cur_bit, f2x, load, overflow, reset_bf, xmm0

ENCODE, DECODE, SKIP = 0x1801AA160, 0x1801A9EF0, 0x1801A8770  # engine.dll
HIGH_LOW_MUL = 0x1803339B0  # server.dll
# x86: cdecl encoder (prop, value, bf, ctx), decoder (prop, bf) -> st(0), skip (prop, bf) in
# engine.dll; HighLowMul (nBits, range) -> st(0) in server.dll. SendProp layout: +0xC nBits,
# +0x10 low, +0x14 high, +0x34 HighLowMul, +0x38 flags (server SendPropFloat 0x102AA690).
X86_ENCODE, X86_DECODE, X86_SKIP = 0x101834D0, 0x101832B0, 0x10181DA0
X86_HIGH_LOW_MUL = 0x102AA1C0
CHANGES_OFTEN = 0x400
BEAM_LOG_VALUES = [-21551.580078, -20535.160156, -20785.0, -18513.0]  # dedicated console log


def f32(x):
    return struct.unpack("<f", struct.pack("<f", x))[0]


def high_low_mul(srv, nbits, low, high):
    if isinstance(srv, X86):
        return srv.call(X86_HIGH_LOW_MUL, 0, [nbits, f2x(f32(high - low))], fret=True)
    call(srv, HIGH_LOW_MUL, nbits, xmm1=f32(high - low))
    return xmm0(srv)


def set_prop(eng, srv, nbits, low, high, flags):
    mul = high_low_mul(srv, nbits, low, high)
    eng.mem_write(PROP, b"\0" * 0x80)
    if isinstance(eng, X86):
        eng.mem_write(PROP + 0xC, struct.pack("<Iff", nbits, low, high))
        eng.mem_write(PROP + 0x34, struct.pack("<fI", mul, flags))
    else:
        eng.mem_write(PROP + 0x14, struct.pack("<Iff", nbits, low, high))
        eng.mem_write(PROP + 0x50, struct.pack("<fI", mul, flags))
    return mul


def encode(eng, f):
    eng.mem_write(BUF, b"\0" * 0x1000)
    reset_bf(eng)
    if isinstance(eng, X86):
        eng.call(X86_ENCODE, 0, [PROP, f2x(f), BF, 0])
    else:
        call(eng, ENCODE, PROP, r8=BF, xmm1=f)
    return cur_bit(eng)


def decode(eng):
    reset_bf(eng)
    if isinstance(eng, X86):
        got = eng.call(X86_DECODE, 0, [PROP, BF], fret=True)
    else:
        got = xmm0(call(eng, DECODE, PROP, rdx=BF))
    return got, cur_bit(eng)


def skip(eng):
    reset_bf(eng)
    if isinstance(eng, X86):
        eng.call(X86_SKIP, 0, [PROP, BF])
    else:
        call(eng, SKIP, PROP, rdx=BF)
    return cur_bit(eng)


def check(eng, srv, label, nbits, low, high, values, flags=CHANGES_OFTEN):
    mul = set_prop(eng, srv, nbits, low, high, flags)
    step = (high - low) / ((1 << nbits) - 1)
    worst, bad = 0.0, []
    for f in values:
        wrote = encode(eng, f)
        got, read = decode(eng)
        skipped = skip(eng)
        err = abs(got - f)
        worst = max(worst, err)
        if wrote != nbits or read != nbits or skipped != nbits or overflow(eng) or err > step:
            bad.append(f"{f} -> {got} (err {err:.5f}; bits w/r/skip {wrote}/{read}/{skipped})")
    print(f"{label}: nBits={nbits} range=[{low:g},{high:g}] HighLowMul={mul:.6f} "
          f"step={step:.7f} max_err={worst:.5f} {'OK' if not bad else 'FAIL'} "
          f"({len(values) - len(bad)}/{len(values)})")
    for line in bad[:10]:
        print("   ", line)
    return len(bad)


def cross(a, b, label, nbits, low, high, values, flags):
    """Encoders of both engines must write identical bits, and each decoder must read the other's
    stream exactly as its own. The two decoders' float arithmetic may differ (x87 vs SSE); that
    difference is reported and must stay below one step."""
    (eng_a, srv_a), (eng_b, srv_b) = a, b
    set_prop(eng_a, srv_a, nbits, low, high, flags)
    set_prop(eng_b, srv_b, nbits, low, high, flags)
    nbytes = (nbits + 7) // 8
    step = (high - low) / ((1 << nbits) - 1)
    bad, diff = [], 0.0
    for f in values:
        encode(eng_a, f)
        encode(eng_b, f)
        bits_a, bits_b = bytes(eng_a.mem_read(BUF, nbytes)), bytes(eng_b.mem_read(BUF, nbytes))
        own_a, own_b = decode(eng_a)[0], decode(eng_b)[0]
        eng_a.mem_write(BUF, bits_b)
        eng_b.mem_write(BUF, bits_a)
        (got_a, read_a), (got_b, read_b) = decode(eng_a), decode(eng_b)
        diff = max(diff, abs(got_a - got_b))
        if (bits_a != bits_b or got_a != own_a or got_b != own_b or read_a != nbits or
                read_b != nbits or abs(got_a - got_b) >= step):
            bad.append(f"{f}: {bits_a.hex()} vs {bits_b.hex()} -> {got_a} / {got_b}")
    print(f"cross {label}: {len(values) - len(bad)}/{len(values)} identical bitstreams, "
          f"cross-decodes OK, max decoder difference {diff:.5f} {'OK' if not bad else 'FAIL'}")
    for line in bad[:10]:
        print("   ", line)
    return len(bad)


PROPS = [  # label, nBits, low, high, flags; stock then expanded
    ("DT_Beam      ", 19, -16384.0, 16384.0, CHANGES_OFTEN),
    ("DT_Beam      ", 20, -32768.0, 32768.0, CHANGES_OFTEN),
    ("DT_EffectData", 17, -16384.0, 16384.0, 0),
    ("DT_EffectData", 18, -32768.0, 32768.0, 0),
]


def run(engine, server, engine2=None, server2=None):
    eng, srv = load(engine), load(server)
    rng = random.Random(25375506)
    common = [0.0, 0.03, -0.03, 1.0, -1.0, 100.25, 16383.9, -16383.9]
    stock_vals = common + [-16384.0, 16384.0] + [rng.uniform(-16384, 16384) for _ in range(2000)]
    wide_vals = (common + BEAM_LOG_VALUES + [-32768.0, 32768.0, 32767.9, -32767.9, 16384.0, -16384.0,
                                              20000.75, -22641.7] +
                 [rng.uniform(-32768, 32768) for _ in range(4000)])
    stock_vals = [f32(v) for v in stock_vals]
    wide_vals = [f32(v) for v in wide_vals]
    out_of_stock = [v for v in BEAM_LOG_VALUES if not -16384.0 <= v <= 16384.0]
    print(f"Logged beam values outside stock +-16384: {len(out_of_stock)}/{len(BEAM_LOG_VALUES)}")
    fails = check(eng, srv, "stock DT_Beam  ", 19, -16384.0, 16384.0, stock_vals)
    fails += check(eng, srv, "expanded DT_Beam", 20, -32768.0, 32768.0, wide_vals)
    fx_vals = [f32(v) for v in (-27904.0, -28164.191406, -28415.960938, 28928.007812, -30208.0)]
    fails += check(eng, srv, "stock EffectData", 17, -16384.0, 16384.0, stock_vals, 0)
    fails += check(eng, srv, "expanded EffectData", 18, -32768.0, 32768.0, wide_vals + fx_vals, 0)
    if engine2:
        other = (load(engine2), load(server2))
        for label, nbits, low, high, flags in PROPS:
            vals = stock_vals if high == 16384.0 else wide_vals + fx_vals
            fails += cross((eng, srv), other, f"{label} {nbits} bits", nbits, low, high, vals, flags)
    return fails


if __name__ == "__main__":
    if len(sys.argv) not in (3, 5):
        sys.exit(__doc__)
    sys.exit(1 if run(*sys.argv[1:]) else 0)
