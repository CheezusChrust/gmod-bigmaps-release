"""Emulate engine.dll's network coordinate codec (read-only on file copies).

Loads an engine.dll copy into the Unicorn CPU emulator and calls its own WriteBitCoord /
ReadBitCoord / WriteBitCoordMP / ReadBitCoordMP machine code, plus the bit-count readers
(SendProp float skip, ReadBitCoordBits, ReadBitCoordMPBits). Reports whether values
round-trip and whether every reader consumes exactly the bits the writer produced.
Addresses are for Steam build 25375506 (P5-01..P5-07). A PE32 (x86)
engine.dll is detected automatically and uses the X86_* addresses; a Linux
engine_srv.so (ELF32, build 25375525) uses the LINUX_* addresses.

Usage: python tools/emulate_codec.py ENGINE_DLL_OR_SO
Requires: pip install unicorn (ELF also needs pyelftools)
"""
import math
import struct
import sys

from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_MODE_64
from unicorn.x86_const import (UC_X86_REG_RCX, UC_X86_REG_RDX, UC_X86_REG_R8, UC_X86_REG_R9,
                               UC_X86_REG_RSP, UC_X86_REG_RAX, UC_X86_REG_XMM0, UC_X86_REG_XMM1,
                               UC_X86_REG_ECX, UC_X86_REG_ESP)

BASE = 0x180000000
WRITE_COORD, READ_COORD = 0x1802B4330, 0x1802B3370
WRITE_MP, READ_MP = 0x1802B4540, 0x1802B3650
FLOAT_SKIP, READ_COORD_BITS, READ_MP_BITS = 0x1801A8770, 0x1802B3540, 0x1802B3890
MASKS, BIT_MASKS = 0x180DB9010, 0x180DB7F10  # runtime-filled tables in .data
STACK, BUF, BF, PROP, STOP = 0x10000000, 0x20000000, 0x20100000, 0x20200000, 0x30000000

# x86 engine.dll (PE32, image base 0x10000000). thiscall (ECX = bit buffer) with stack
# arguments; float results in x87 st(0). The x86 codec uses no runtime-filled tables.
X86_WRITE_COORD, X86_READ_COORD = 0x10261330, 0x10260490
X86_WRITE_MP, X86_READ_MP = 0x10261540, 0x10260740
X86_FLOAT_SKIP, X86_READ_COORD_BITS, X86_READ_MP_BITS = 0x10181DA0, 0x10260650, 0x10260930
X86_STACK = 0x08000000  # STACK overlaps the x86 image; BUF/BF/PROP/STOP are shared
X86_PROP_FLAGS = 0x38   # SendProp flags offset (x64: 0x54)
FSTP_OUT = STOP + 0x100


class X86:
    """32-bit engine image in Unicorn with the same helper interface as the x64 path."""

    def __init__(self, data):
        pe = struct.unpack_from("<I", data, 0x3C)[0]
        count = struct.unpack_from("<H", data, pe + 6)[0]
        opt = struct.unpack_from("<H", data, pe + 20)[0]
        base, size_image = struct.unpack_from("<I", data, pe + 24 + 28)[0], struct.unpack_from("<I", data, pe + 24 + 56)[0]
        self.mu = Uc(UC_ARCH_X86, UC_MODE_32)
        self.mu.mem_map(base, (size_image + 0xFFF) & ~0xFFF)
        self.mu.mem_write(base, data[:0x1000])
        for i in range(count):
            o = pe + 24 + opt + 40 * i
            _, va, rsize, raw = struct.unpack_from("<IIII", data, o + 8)
            self.mu.mem_write(base + va, data[raw:raw + rsize])
        for addr, size in ((X86_STACK, 0x100000), (BUF, 0x1000), (BF, 0x1000), (PROP, 0x1000), (STOP, 0x1000)):
            self.mu.mem_map(addr, size)
        # STOP: hlt. STOP+0x10: fninit; hlt. STOP+0x20: fstp dword [FSTP_OUT]; hlt.
        self.mu.mem_write(STOP, b"\xF4")
        self.mu.mem_write(STOP + 0x10, b"\xDB\xE3\xF4")
        self.mu.mem_write(STOP + 0x20, b"\xD9\x1D" + struct.pack("<I", FSTP_OUT) + b"\xF4")

    def mem_write(self, addr, data):
        self.mu.mem_write(addr, data)

    def mem_read(self, addr, size):
        return self.mu.mem_read(addr, size)

    def call(self, fn, ecx=0, args=(), fret=False):
        self.mu.emu_start(STOP + 0x10, STOP + 0x12)  # empty x87 stack before every call
        esp = X86_STACK + 0xF0000
        ret = STOP + 0x20 if fret else STOP
        self.mu.mem_write(esp, b"".join(struct.pack("<I", w & 0xFFFFFFFF) for w in [ret, *args]))
        self.mu.reg_write(UC_X86_REG_ESP, esp)
        self.mu.reg_write(UC_X86_REG_ECX, ecx)
        self.mu.emu_start(fn, STOP + 0x26 if fret else STOP, count=100000)
        if fret:
            return struct.unpack("<f", self.mu.mem_read(FSTP_OUT, 4))[0]

    def reset_bf(self, bits):
        self.mu.mem_write(BF, struct.pack("<IIIIB", BUF, 0x1000, 0x8000, bits, 0) + b"\0" * 11)

    def cur_bit(self):
        return struct.unpack("<I", self.mu.mem_read(BF + 0xC, 4))[0]

    def overflow(self):
        return self.mu.mem_read(BF + 0x10, 1)[0]

    def write(self, f, mp):
        if mp is None:
            self.call(X86_WRITE_COORD, BF, [f2x(f)])
        else:
            self.call(X86_WRITE_MP, BF, [f2x(f), mp[0], mp[1]])

    def read(self, mp):
        if mp is None:
            return self.call(X86_READ_COORD, BF, fret=True)
        return self.call(X86_READ_MP, BF, [mp[0], mp[1]], fret=True)

    def skip(self, flags):
        self.mu.mem_write(PROP + X86_PROP_FLAGS, struct.pack("<I", flags))
        self.call(X86_FLOAT_SKIP, 0, [PROP, BF])

    def cmp_bits(self, mp):
        if mp is None:
            self.call(X86_READ_COORD_BITS, BF)
        else:
            self.call(X86_READ_MP_BITS, BF, [mp[0], mp[1]])


# Linux engine_srv.so (ELF32, GCC, build 25375525). cdecl: `this` is the first stack
# argument; float results in x87 st(0). Loaded at LINUX_BASE with its R_386_RELATIVE / R_386_32 relocations
# applied (the codec functions call nothing outside the image).
LINUX_BASE = 0x40000000
LINUX_WRITE_COORD, LINUX_READ_COORD = 0x20C440, 0x20D810
LINUX_WRITE_MP, LINUX_READ_MP = 0x20C250, 0x20D9D0
LINUX_FLOAT_SKIP, LINUX_READ_COORD_BITS, LINUX_READ_MP_BITS = 0x1266A0, 0x20DC30, 0x20DD10


class LinuxX86(X86):
    def __init__(self, data):
        import os
        sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
        from elf_image import ElfImage
        img = ElfImage(data)
        b = LINUX_BASE
        size = max(va + n for va, n, _, _, _ in img.secs)
        self.mu = Uc(UC_ARCH_X86, UC_MODE_32)
        self.mu.mem_map(b, (size + 0xFFF) & ~0xFFF)
        for va, _, raw, rsize, _ in img.secs:
            if rsize:
                self.mu.mem_write(b + va, data[raw:raw + rsize])
        for va, (t, _, s, a) in img.relocs.items():
            if t == 8 or (t == 1 and s is not None):
                self.mu.mem_write(b + va, struct.pack("<I", (img.reloc_value(va) + b) & 0xFFFFFFFF))
        for addr, size in ((X86_STACK, 0x100000), (BUF, 0x1000), (BF, 0x1000), (PROP, 0x1000), (STOP, 0x1000)):
            self.mu.mem_map(addr, size)
        self.mu.mem_write(STOP, b"\xF4")
        self.mu.mem_write(STOP + 0x10, b"\xDB\xE3\xF4")
        self.mu.mem_write(STOP + 0x20, b"\xD9\x1D" + struct.pack("<I", FSTP_OUT) + b"\xF4")

    def write(self, f, mp):
        b = LINUX_BASE
        if mp is None:
            self.call(b + LINUX_WRITE_COORD, 0, [BF, f2x(f)])
        else:
            self.call(b + LINUX_WRITE_MP, 0, [BF, f2x(f), mp[0], mp[1]])

    def read(self, mp):
        b = LINUX_BASE
        if mp is None:
            return self.call(b + LINUX_READ_COORD, 0, [BF], fret=True)
        return self.call(b + LINUX_READ_MP, 0, [BF, mp[0], mp[1]], fret=True)

    def skip(self, flags):
        self.mu.mem_write(PROP + X86_PROP_FLAGS, struct.pack("<I", flags))
        self.call(LINUX_BASE + LINUX_FLOAT_SKIP, 0, [PROP, BF])

    def cmp_bits(self, mp):
        b = LINUX_BASE
        if mp is None:
            self.call(b + LINUX_READ_COORD_BITS, 0, [BF])
        else:
            self.call(b + LINUX_READ_MP_BITS, 0, [BF, mp[0], mp[1]])


def load(path):
    data = open(path, "rb").read()
    if data[:4] == b"\x7fELF":
        return LinuxX86(data)
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    if struct.unpack_from("<H", data, pe + 24)[0] == 0x10B:
        return X86(data)
    count = struct.unpack_from("<H", data, pe + 6)[0]
    opt = struct.unpack_from("<H", data, pe + 20)[0]
    size_image = struct.unpack_from("<I", data, pe + 24 + 56)[0]
    mu = Uc(UC_ARCH_X86, UC_MODE_64)
    mu.mem_map(BASE, (size_image + 0xFFF) & ~0xFFF)
    mu.mem_write(BASE, data[:0x1000])
    for i in range(count):
        o = pe + 24 + opt + 40 * i
        _, va, rsize, raw = struct.unpack_from("<IIII", data, o + 8)
        mu.mem_write(BASE + va, data[raw:raw + rsize])
    for n in range(33):
        mu.mem_write(MASKS + 4 * n, struct.pack("<I", (1 << n) - 1 if n < 32 else 0xFFFFFFFF))
    for n in range(32):
        mu.mem_write(BIT_MASKS + 4 * n, struct.pack("<I", 1 << n))
    for addr, size in ((STACK, 0x100000), (BUF, 0x1000), (BF, 0x1000), (PROP, 0x1000), (STOP, 0x1000)):
        mu.mem_map(addr, size)
    mu.mem_write(STOP, b"\xF4")
    return mu


def f2x(f):
    return int.from_bytes(struct.pack("<f", f), "little")


def call(mu, fn, rcx, rdx=0, r8=0, r9=0, xmm1=None):
    rsp = STACK + 0xF0000
    mu.mem_write(rsp, struct.pack("<Q", STOP))
    mu.reg_write(UC_X86_REG_RSP, rsp)
    for reg, val in ((UC_X86_REG_RCX, rcx), (UC_X86_REG_RDX, rdx), (UC_X86_REG_R8, r8), (UC_X86_REG_R9, r9)):
        mu.reg_write(reg, val)
    if xmm1 is not None:
        mu.reg_write(UC_X86_REG_XMM1, f2x(xmm1))
    mu.emu_start(fn, STOP, count=100000)
    return mu


def reset_bf(mu, bits=0):
    if isinstance(mu, X86):
        return mu.reset_bf(bits)
    mu.mem_write(BF, struct.pack("<QIIB", BUF, 0x1000, 0x8000, 0) + b"\0" * 8)
    mu.mem_write(BF + 0x10, struct.pack("<I", bits))


def cur_bit(mu):
    if isinstance(mu, X86):
        return mu.cur_bit()
    return struct.unpack("<I", mu.mem_read(BF + 0x10, 4))[0]


def overflow(mu):
    if isinstance(mu, X86):
        return mu.overflow()
    return mu.mem_read(BF + 0x14, 1)[0]


def xmm0(mu):
    return struct.unpack("<f", (mu.reg_read(UC_X86_REG_XMM0) & 0xFFFFFFFF).to_bytes(4, "little"))[0]


def expected(f, res):
    q = math.floor(abs(f) * res) / res
    return -q if f < 0 and q else q


def f32(f):
    return struct.unpack("<f", struct.pack("<f", f))[0]


# Independent bit-count model for the expanded widths (15 integer bits, 5/3 fraction bits;
# MP keeps 11 integer bits while |int| < 2048). Structure follows the public SDK bitbuf.
def model_bits(f, mp):
    a = abs(f32(f))
    intval = int(a)
    if mp is None:
        frac = int(a * 32) & 31
        return 2 + ((1 + (15 if intval else 0) + (5 if frac else 0)) if intval or frac else 0)
    integral, low = mp
    ibits = 11 if intval < 2048 else 15
    if integral:
        return 3 + ibits if intval else 2
    fbits = 3 if low else 5
    return 3 + (ibits if intval else 0) + fbits


MODES = [("COORD", None, 32)] + [(f"MP{'-int' if i else ''}{'-low' if l else ''}", (i, l), 1 if i else (8 if l else 32))
                                 for i in (0, 1) for l in (0, 1) if not (i and l)]


def write(mu, f, mp):
    if isinstance(mu, X86):
        return mu.write(f, mp)
    if mp is None:
        call(mu, WRITE_COORD, BF, xmm1=f)
    else:
        call(mu, WRITE_MP, BF, r8=mp[0], r9=mp[1], xmm1=f)


def read(mu, mp):
    if isinstance(mu, X86):
        return mu.read(mp)
    if mp is None:
        call(mu, READ_COORD, BF)
    else:
        call(mu, READ_MP, BF, rdx=mp[0], r8=mp[1])
    return xmm0(mu)


def skip(mu, mp):
    flags = 2 if mp is None else (0x2000, 0x4000, 0x8000)[(mp[1] and 1) or (mp[0] and 2)]
    if isinstance(mu, X86):
        return mu.skip(flags)
    mu.mem_write(PROP + 0x54, struct.pack("<I", flags))
    call(mu, FLOAT_SKIP, PROP, rdx=BF)


def cmp_bits(mu, mp):
    if isinstance(mu, X86):
        return mu.cmp_bits(mp)
    if mp is None:
        call(mu, READ_COORD_BITS, BF)
    else:
        call(mu, READ_MP_BITS, BF, rdx=mp[0], r8=mp[1])


def get_bits(buf, start, n):
    v = int.from_bytes(buf, "little")
    return (v >> start) & ((1 << n) - 1)


def sequence(mu, vals, mp, res, start, pattern):
    """Write vals back to back from bit `start` over a pre-filled buffer, then read, skip
    and bit-count them in sequence. Returns a list of problems."""
    probs = []
    mu.mem_write(BUF, pattern)
    reset_bf(mu, start)
    ends = []
    for f in vals:
        write(mu, f, mp)
        ends.append(cur_bit(mu))
    if overflow(mu):
        probs.append("write overflow")
    buf = bytes(mu.mem_read(BUF, 0x40))
    if start and get_bits(buf, 0, start) != get_bits(pattern, 0, start):
        probs.append(f"bits before start {start} changed")
    pos = start
    for f, end in zip(vals, ends):
        want_n = model_bits(f, mp)
        if end - pos != want_n:
            probs.append(f"{f}: wrote {end - pos} bits, model {want_n}")
        pos = end
    for label, step in (("read", read), ("skip", skip), ("cmp", cmp_bits)):
        reset_bf(mu, start)
        for f, end in zip(vals, ends):
            got = step(mu, mp)
            if cur_bit(mu) != end:
                probs.append(f"{label} {f}: at bit {cur_bit(mu)}, writer ended at {end}")
                break
            if label == "read" and abs(got - expected(f32(f), res)) > 1e-3:
                probs.append(f"read {f} -> {got} (want {expected(f32(f), res)})")
        if overflow(mu):
            probs.append(f"{label} overflow")
    return probs


def run_extended(path):
    """Endpoints, fractional neighbours, nonzero start offsets and packed sequences."""
    mu = load(path)
    edges = [32768.0, -32768.0, 32768.5, -32768.5, 32768.96875, -32768.96875, 32767.96875,
             -32767.96875, 32767.5, 16383.96875, -16383.96875, 16384.03125, -16384.03125,
             2047.96875, -2047.96875, 2048.0, 2048.03125, -2048.03125, 0.03125, -0.03125,
             0.125, -0.125, 0.96875, 1.03125]
    offsets = [0, 1, 3, 5, 7, 8, 13, 31, 32, 33, 63]
    seqs = [[32768.0, -32768.0, 0.5], [-32768.96875, 16384.03125, 2047.5, 0.0],
            [32767.96875, -16383.96875, 2048.0, -1.0, 32768.5]]
    patterns = [bytes([0xA5, 0x3C, 0xFF, 0x00] * 16), b"\xFF" * 64]
    failures = 0
    print("\nExtended checks (independent bit model; writer/read/skip/cmp agreement)")
    for name, mp, res in MODES:
        probs, cases = [], 0
        for f in edges:
            for off in offsets:
                for pat in patterns:
                    cases += 1
                    probs += [f"[{f} @bit {off}] {p}" for p in sequence(mu, [f], mp, res, off, pat)]
        for s in seqs:
            for off in (0, 5, 29):
                cases += 1
                probs += [f"[seq @bit {off}] {p}" for p in sequence(mu, s, mp, res, off, patterns[0])]
        failures += len(probs)
        print(f"{name:12s} {'OK' if not probs else 'FAIL'} ({cases} cases, {len(probs)} problems)")
        for line in probs[:20]:
            print("   ", line)
    print("\nInformational, beyond the +/-32768.97 design range (not counted as failures):")
    for f in (32769.0, -32769.0, 40000.0):
        mu.mem_write(BUF, b"\0" * 0x40)
        reset_bf(mu)
        write(mu, f, None)
        n = cur_bit(mu)
        reset_bf(mu)
        got = read(mu, None)
        print(f"    COORD {f} -> {got} ({n} bits written, {cur_bit(mu)} read)")
    return failures


def run(path):
    mu = load(path)
    values = [0.0, 0.5, 1.0, -1.0, 100.25, 2047.0, 2048.0, -2048.5, 16383.0, 16384.0, 16385.5,
              -16385.0, 20000.75, -22641.7, 16703.24, 30000.0, 32767.0, -32767.97, -32768.0]
    modes = [("COORD", None, 32)] + [(f"MP{'-int' if i else ''}{'-low' if l else ''}", (i, l), 1 if i else (8 if l else 32))
                                      for i in (0, 1) for l in (0, 1) if not (i and l)]
    failures = 0
    for name, mp, res in modes:
        bad = []
        for f in values:
            mu.mem_write(BUF, b"\0" * 0x1000)
            reset_bf(mu)
            write(mu, f, mp)
            nbits = cur_bit(mu)
            reset_bf(mu)
            got = read(mu, mp)
            used = cur_bit(mu)
            # Bit-count readers must consume exactly nbits as well.
            counts = []
            reset_bf(mu)
            skip(mu, mp)
            counts.append(cur_bit(mu))
            reset_bf(mu)
            cmp_bits(mu, mp)
            counts.append(cur_bit(mu))
            want = expected(f, res)
            if abs(got - want) > 1e-3 or used != nbits or any(c != nbits for c in counts) or overflow(mu):
                bad.append(f"{f} -> {got} (want {want}; bits w/r/skip/cmp {nbits}/{used}/{counts[0]}/{counts[1]})")
        failures += len(bad)
        print(f"{name:12s} {'OK' if not bad else 'FAIL'} ({len(values) - len(bad)}/{len(values)})")
        for line in bad:
            print("   ", line)
    return failures


if __name__ == "__main__":
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    base_failures = run(sys.argv[1])
    sys.exit(1 if base_failures + run_extended(sys.argv[1]) else 0)
