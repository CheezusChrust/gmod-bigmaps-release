"""Re-locate the bigger-maps patch sites in a new GMod or vbsp++ build without Ghidra.

gen    Refresh masked byte signatures for the reference build using the bundled site
       metadata. Requires exact reference binary hashes. Data sites are anchored on a
       code instruction that references them, found automatically.
find   Search a new build for those signatures, verify the original bytes/values at each hit,
       and print patch-script entries plus reloc-free +-32768.0f redirect candidates.
xrefs  List code that references VA (RIP-relative/image-relative operands, call/jmp targets;
       on PE32 relocated absolute operands), with the containing function start. Replaces
       Ghidra cross-references for porting. --data also lists relocated data pointers to VA.
       Also reads 32-bit Linux ELF (.so): relocated absolute operands, GOT-relative operands,
       direct and relocated (R_386_PC32) branches, named by the symbol table (elf_image.py).
sym    ELF only: list symbols whose mangled name contains TEXT, or name the symbol at a hex VA.

Usage:
  python tools/port_sites.py gen   ROOT OUT.json [--target gmod|gmod-x86|vbsppp]   (default gmod)
  python tools/port_sites.py find  ROOT SITES.json [--json REPORT.json]
  python tools/port_sites.py xrefs FILE VA [--data]
  python tools/port_sites.py sym   FILE TEXT|VA

ROOT has the install layout: bin/win64/engine.dll, garrysmod/bin/win64/{client,server}.dll
(gmod), bin/engine.dll, garrysmod/bin/{client,server}.dll (gmod-x86) or bin/win64/vbsp++.exe
(vbsppp). PE32+ (x64) and PE32 (x86) images are supported. PE32 has no .pdata: instruction and
function starts come from a linear sweep of .text that resyncs at relocated code pointers and
treats relocated dwords outside instructions as data (MSVC jump tables); function starts are
call targets, exports and code pointers from outside jump tables (a heuristic: "fn" may be a
code label inside a function).
Read-only: never writes to any PE file. See tools/README.md.
"""
import bisect
import hashlib
import json
import os
import re
import struct
import sys
import tempfile

import capstone
import numpy as np
from capstone import x86

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from elf_image import ElfImage, is_elf  # noqa: E402

# Data sites have no code around them: they are found through one code instruction that
# references them. gen uses these build-25375506 anchors when they still reference the site
# (keeps that JSON stable); otherwise it picks a referencing instruction automatically.
REFS = {
    "P4-03": 0x18021C58F,  # s_PartitionMin reader, FUN_18021c420
    "P4-04": 0x18021C5B9,  # s_PartitionMax reader, same function
    "P4-05": 0x1802191DB,  # +16379 ray clamp, FUN_1802191c0
    "P4-06": 0x1802191C9,  # -16379 ray clamp, same function
    "P4-12": 0x18003CF41,  # client: r_mapextents registration loads default string
    "P4-13": 0x1802E771A,  # server: k_flMaxEntityPosCoord read by Bad SetLocalOrigin check
    "P4-14": 0x1800A1686,  # client: k_flMaxEntityPosCoord reader
    "P5-04": 0x1802B35E0,  # ReadBitCoordBits table
    "P5-06b": 0x1802B37AA,  # ReadBitCoordMP numbits table
    "P5-07b": 0x1802B3948,  # ReadBitCoordMPBits numbits table
}
# ConVar default strings: (convar name, how the default-value pointer is passed at registration:
# a register loaded by LEA on x64, or "push" = the relocated push just before the name push on x86).
CVARS = {"P4-12": ("r_mapextents", "r8")}
CVARS_X86 = {"P4-12": ("r_mapextents", "push")}
MAX_INSNS = 24

MD64 = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
MD64.detail = True
MD32 = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
MD32.detail = True
MD32_LITE = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
md = MD64


class PE:
    def __init__(self, path):
        self.path = path
        self.data = d = open(path, "rb").read()
        self.sha = hashlib.sha256(d).hexdigest().upper()
        pe = struct.unpack_from("<I", d, 0x3C)[0]
        n = struct.unpack_from("<H", d, pe + 6)[0]
        osz = struct.unpack_from("<H", d, pe + 20)[0]
        opt = pe + 24
        magic = struct.unpack_from("<H", d, opt)[0]
        if magic not in (0x10B, 0x20B):
            sys.exit(f"{path}: not a PE32 or PE32+ image")
        self.is32 = magic == 0x10B
        self.md = MD32 if self.is32 else MD64
        if self.is32:
            self.base = struct.unpack_from("<I", d, opt + 28)[0]
        else:
            self.base = struct.unpack_from("<Q", d, opt + 24)[0]
        self.secs = []
        for i in range(n):
            o = opt + osz + 40 * i
            name = d[o:o + 8].rstrip(b"\0").decode(errors="replace")
            vs, va, rs, raw = struct.unpack_from("<IIII", d, o + 8)
            ch = struct.unpack_from("<I", d, o + 36)[0]
            self.secs.append((name, va, vs, raw, rs, ch))
        dd = opt + (96 if self.is32 else 112)
        self.fix = set()  # RVAs of every byte covered by a base relocation
        self.fixstarts = {}  # RVA where a relocated value starts -> its size
        rva, size = struct.unpack_from("<II", d, dd + 5 * 8)
        o = self.r2o(rva)
        end = o + size if o is not None else 0
        while o is not None and o < end:
            page, bs = struct.unpack_from("<II", d, o)
            if bs < 8:
                break
            for k in range((bs - 8) // 2):
                e = struct.unpack_from("<H", d, o + 8 + 2 * k)[0]
                # DIR64 (type 10) on x64; HIGHLOW (type 3) on x86
                width = {10: 8, 3: 4}.get(e >> 12) if self.is32 else (8 if e >> 12 == 10 else None)
                if width:
                    self.fix.update(page + (e & 0xFFF) + b for b in range(width))
                    self.fixstarts[page + (e & 0xFFF)] = width
            o += bs
        self.funcs = []
        self._sweep_done = False
        if not self.is32:
            rva, size = struct.unpack_from("<II", d, dd + 3 * 8)
            o = self.r2o(rva)
            for k in range(size // 12 if o is not None else 0):
                b, e, u = struct.unpack_from("<III", d, o + 12 * k)
                self.funcs.append((b, e, u))
            self.funcs.sort()
            self.fbegins = [f[0] for f in self.funcs]
        else:
            self.export_rvas = set()
            rva, size = struct.unpack_from("<II", d, dd)
            o = self.r2o(rva) if size else None
            if o is not None:
                nfun = struct.unpack_from("<I", d, o + 20)[0]
                ao = self.r2o(struct.unpack_from("<I", d, o + 28)[0])
                for k in range(nfun if ao is not None else 0):
                    self.export_rvas.add(struct.unpack_from("<I", d, ao + 4 * k)[0])
            self.export_rvas.add(struct.unpack_from("<I", d, opt + 16)[0])  # entry point

    def r2o(self, rva):
        for _, va, vs, raw, rs, _ in self.secs:
            if va <= rva < va + rs:
                return raw + rva - va
        return None

    def section_of(self, va):
        rva = va - self.base
        for s in self.secs:
            if s[1] <= rva < s[1] + max(s[2], s[4]):
                return s
        return None

    def is_exec(self, va):
        rva = va - self.base
        return any(sva <= rva < sva + max(vs, rs) and ch & 0x20000000
                   for _, sva, vs, raw, rs, ch in self.secs)

    def read(self, va, n):
        o = self.r2o(va - self.base)
        return None if o is None else self.data[o:o + n]

    def text(self):
        for name, va, vs, raw, rs, ch in self.secs:
            if name == ".text":
                return self.base + va, self.data[raw:raw + rs]
        sys.exit(f"{self.path}: no .text")

    def dword(self, va):
        b = self.read(va, 4)
        return None if b is None or len(b) < 4 else struct.unpack("<I", b)[0]

    def _sweep(self):
        """PE32: linear sweep of .text -> instruction starts, data dwords and function starts."""
        if self._sweep_done:
            return
        self._sweep_done = True
        text_va, text = self.text()
        n = len(text)
        trva = text_va - self.base
        cache = os.path.join(tempfile.gettempdir(), f"port_sites_sweep_{self.sha[:16]}.npz")
        if os.path.exists(cache):
            z = np.load(cache)
            self.starts, self.size_at = z["starts"], z["size_at"]
            self.data_fix = set(z["data_fix"].tolist())
            self.fstarts = z["fstarts"].tolist()
            return
        fixset = {r - trva for r in self.fixstarts if trva <= r < trva + n}
        ptr_src = {}
        for r in self.fixstarts:
            v = self.dword(self.base + r)
            if v is not None and text_va <= v < text_va + n:
                ptr_src.setdefault(v - text_va, []).append(r)
        bounds = sorted(set(ptr_src) | {r - trva for r in self.export_rvas if trva <= r < trva + n})
        starts = np.zeros(n, dtype=bool)
        size_at = np.zeros(n, dtype=np.uint8)
        data_fix = set()
        pos, bi = 0, 0
        while pos < n:
            if pos in fixset:
                data_fix.add(pos)
                pos += 4
                continue
            while bi < len(bounds) and bounds[bi] <= pos:
                bi += 1
            nb = bounds[bi] if bi < len(bounds) else n
            end = pos
            for a, sz, _, _ in MD32_LITE.disasm_lite(text[pos:min(n, nb + 16)], pos):
                if a + sz > nb:
                    end = nb  # runs into a known code start: resync there
                    break
                if a in fixset:
                    end = a  # a relocated dword where an instruction would start: data
                    break
                starts[a] = True
                size_at[a] = sz
                end = a + sz
            else:
                if end < nb:
                    end += 1  # undecodable byte
            pos = end
        fs = set()
        for a in np.nonzero(starts & (np.frombuffer(text, dtype=np.uint8) == 0xE8))[0].tolist():
            if size_at[a] == 5:
                t = a + 5 + struct.unpack_from("<i", text, a + 1)[0]
                if 0 <= t < n and starts[t]:
                    fs.add(t)
        for t, srcs in ptr_src.items():
            if starts[t] and t not in fixset and any(s - trva not in data_fix for s in srcs):
                fs.add(t)
        fs.update(r - trva for r in self.export_rvas if trva <= r < trva + n and starts[r - trva])
        self.starts, self.size_at, self.data_fix = starts, size_at, data_fix
        self.fstarts = sorted(fs)
        np.savez(cache, starts=starts, size_at=size_at, data_fix=np.array(sorted(data_fix), dtype=np.int64),
                 fstarts=np.array(self.fstarts, dtype=np.int64))

    def ins_start(self, va):
        """PE32: start of the swept instruction covering VA, or None."""
        self._sweep()
        text_va, text = self.text()
        off = va - text_va
        for k in range(16):
            p = off - k
            if 0 <= p < len(self.starts) and self.starts[p]:
                return text_va + p if p + int(self.size_at[p]) > off else None
        return None

    def func_at(self, va):
        """Range of the .pdata entry (possibly a chained fragment) containing VA.
        PE32: from the nearest heuristic function start to the next one."""
        if self.is32:
            self._sweep()
            text_va, text = self.text()
            off = va - text_va
            if not 0 <= off < len(text):
                return None
            i = bisect.bisect_right(self.fstarts, off) - 1
            if i < 0:
                return None
            e = self.fstarts[i + 1] if i + 1 < len(self.fstarts) else len(text)
            return text_va + self.fstarts[i], text_va + e
        rva = va - self.base
        i = bisect.bisect_right(self.fbegins, rva) - 1
        if i < 0 or not (self.funcs[i][0] <= rva < self.funcs[i][1]):
            return None
        b, e, _ = self.funcs[i]
        return self.base + b, self.base + e

    def func_start(self, va):
        """Primary function start: follow UNW_FLAG_CHAININFO links from the fragment's unwind info."""
        if self.is32:
            f = self.func_at(va)
            return f[0] if f else None
        rva = va - self.base
        i = bisect.bisect_right(self.fbegins, rva) - 1
        if i < 0 or not (self.funcs[i][0] <= rva < self.funcs[i][1]):
            return None
        b, _, u = self.funcs[i]
        for _ in range(16):
            o = self.r2o(u)
            if o is None or not (self.data[o] >> 3) & 4:
                break
            n = self.data[o + 2]
            b, _, u = struct.unpack_from("<III", self.data, o + 4 + 2 * ((n + 1) & ~1))
        return self.base + b

    def insns(self, va):
        f = self.func_at(va)
        if not f:
            raise ValueError(f"{va:#x} is not inside any .pdata function range")
        if not self.is32:
            return list(md.disasm(self.read(f[0], f[1] - f[0]), f[0]))
        # PE32: the contiguous run of swept instructions (no data gap) inside f that holds VA
        text_va, _ = self.text()
        runs, run, a = [], [], f[0]
        while a < f[1]:
            off = a - text_va
            if not self.starts[off]:
                if run:
                    runs.append(run)
                    run = []
                a += 1
                continue
            ins = next(self.md.disasm(self.read(a, 16), a), None)
            if ins is None or ins.size != self.size_at[off]:
                raise ValueError(f"{a:#x}: decode disagrees with the sweep")
            run.append(ins)
            a += ins.size
        if run:
            runs.append(run)
        for r in runs:
            if r[0].address <= va < r[-1].address + r[-1].size:
                return r
        raise ValueError(f"{va:#x} is not inside swept code of function {f[0]:#x}")


def rip_target(ins):
    for op in ins.operands:
        if op.type == x86.X86_OP_MEM and op.mem.base == x86.X86_REG_RIP:
            return ins.address + ins.size + op.mem.disp
    return None


def rva_disp(ins):
    """Large disp32 on a non-RIP operand: image-base-relative table access (base reg = image base)."""
    for op in ins.operands:
        if (op.type == x86.X86_OP_MEM and op.mem.base not in (x86.X86_REG_RIP, x86.X86_REG_RSP, x86.X86_REG_RBP)
                and op.mem.disp >= 0x10000):
            return op.mem.disp
    return None


def abs_target(pe, ins):
    """PE32: value of the first relocated dword inside INS (an absolute address operand)."""
    r = ins.address - pe.base
    for k in range(ins.size):
        if pe.fixstarts.get(r + k) == 4:
            return pe.dword(ins.address + k)
    return None


def ref_target(pe, ins):
    if pe.is32:
        return abs_target(pe, ins)
    t = rip_target(ins)
    if t is None and rva_disp(ins) is not None:
        t = pe.base + rva_disp(ins)
    return t


def code_ref(pe, ins):
    """Address read by a code site's memory operand: RIP-relative on x64, relocated abs32 on x86."""
    return abs_target(pe, ins) if pe.is32 else rip_target(ins)


def branch_target(ins):
    if capstone.CS_GRP_JUMP in ins.groups or capstone.CS_GRP_CALL in ins.groups:
        for op in ins.operands:
            if op.type == x86.X86_OP_IMM:
                return op.imm
    return None


def find_xrefs32(pe, target, branches=True):
    """PE32: instructions holding a relocated absolute operand equal to TARGET (memory operand or
    immediate such as push offset) or, optionally, a call/jmp/jcc rel32 to TARGET."""
    pe._sweep()
    text_va, text = pe.text()
    trva = text_va - pe.base
    cand = {}
    for r, w in pe.fixstarts.items():
        if w == 4 and trva <= r < trva + len(text) and r - trva not in pe.data_fix \
                and pe.dword(pe.base + r) == target:
            cand[r - trva] = "abs"
    if branches:
        b = np.frombuffer(text, dtype=np.uint8).astype(np.int64)
        n = len(b) - 3
        d = b[:n] | (b[1:n + 1] << 8) | (b[2:n + 2] << 16) | (b[3:n + 3] << 24)
        d = np.where(d >= 1 << 31, d - (1 << 32), d)
        pos = np.arange(n, dtype=np.int64) + text_va + 4
        for p in np.nonzero(d + pos == target)[0].tolist():
            cand.setdefault(p, "rel")
    out = {}
    for p, kind in sorted(cand.items()):
        s = pe.ins_start(text_va + p)
        if s is None or s == text_va + p:
            continue
        ins = next(pe.md.disasm(pe.read(s, 16), s), None)
        if ins is None:
            continue
        k = text_va + p - s
        if kind == "abs" or (ins.imm_size == 4 and ins.imm_offset == k and branch_target(ins) == target):
            out[s] = ins
    return [out[a] for a in sorted(out)]


def data_pointers(pe, target):
    """Relocated pointer-sized values equal to TARGET outside code instructions (tables, vtables,
    .data/.rdata structures), with their section names."""
    if pe.is32:
        pe._sweep()
        text_va, _ = pe.text()
    out = []
    for r, w in sorted(pe.fixstarts.items()):
        v = pe.read(pe.base + r, w)
        if v is None or len(v) < w or int.from_bytes(v, "little") != target:
            continue
        sec = pe.section_of(pe.base + r)
        if pe.is32 and sec and sec[0] == ".text" and pe.base + r - text_va not in pe.data_fix:
            continue  # an instruction operand, already listed as a code reference
        if not pe.is32 and sec and sec[5] & 0x20000000:
            continue
        out.append((pe.base + r, sec[0] if sec else "?"))
    return out


def find_xrefs(pe, target, branches=True):
    """Instructions whose RIP-relative operand, image-relative disp32 or (optionally) call/jmp
    rel32 resolves to TARGET. Brute-force over every .text offset, so leaf functions without
    .pdata are covered; hits inside .pdata functions are confirmed by linear disassembly."""
    if pe.is32:
        return find_xrefs32(pe, target, branches)
    text_va, text = pe.text()
    b = np.frombuffer(text, dtype=np.uint8).astype(np.int64)
    n = len(b) - 3
    d = b[:n] | (b[1:n + 1] << 8) | (b[2:n + 2] << 16) | (b[3:n + 3] << 24)
    d = np.where(d >= 1 << 31, d - (1 << 32), d)
    pos = np.arange(n, dtype=np.int64) + text_va + 4
    cand = set()
    for imm in (0, 1, 2, 4):  # bytes of immediate after the disp32
        cand.update(np.nonzero(d + pos + imm == target)[0].tolist())
    cand.update(np.nonzero(d == target - pe.base)[0].tolist())
    out, linear = {}, {}
    for p in sorted(cand):
        a = text_va + p
        for k in range(1, 12):
            ins = next(md.disasm(text[p - k:p - k + 16], a - k), None) if p >= k else None
            if ins is None or ins.address + ins.size < a + 4:
                continue
            hit = ((ins.disp_offset == k and ref_target(pe, ins) == target)
                   or (branches and ins.imm_size == 4 and ins.imm_offset == k and branch_target(ins) == target))
            if not hit:
                continue
            f = pe.func_at(ins.address)
            if f:
                if f not in linear:
                    linear[f] = {i.address for i in md.disasm(pe.read(f[0], f[1] - f[0]), f[0])}
                if ins.address not in linear[f]:
                    continue
            out[ins.address] = ins
            break
    return [out[a] for a in sorted(out)]


THUNK_REGS = {"__x86.get_pc_thunk.bx": x86.X86_REG_EBX, "__i686.get_pc_thunk.bx": x86.X86_REG_EBX,
              "__x86.get_pc_thunk.cx": x86.X86_REG_ECX, "__x86.get_pc_thunk.ax": x86.X86_REG_EAX,
              "__x86.get_pc_thunk.dx": x86.X86_REG_EDX, "__x86.get_pc_thunk.si": x86.X86_REG_ESI,
              "__x86.get_pc_thunk.di": x86.X86_REG_EDI, "__x86.get_pc_thunk.bp": x86.X86_REG_EBP}


class ElfCode:
    """Per-function linear disassembly of an ElfImage (GCC keeps jump tables out of .text) and the
    registers each function loads with the GOT address (`call get_pc_thunk.REG; add REG, imm`)."""

    def __init__(self, elf):
        self.elf = elf
        self.cache = {}
        self.thunks = {v: THUNK_REGS[n] for v, _, n, _, d in elf.symbols if d and n in THUNK_REGS}

    def function(self, va):
        f = self.elf.func_at(va)
        if f is None:
            return None, None, set()
        if f[0] not in self.cache:
            insns = list(MD32.disasm(self.elf.read(f[0], f[1] - f[0]), f[0]))
            got_regs = set()
            for a, b in zip(insns, insns[1:]):
                t = branch_target(a) if a.mnemonic == "call" else None
                if t in self.thunks and b.mnemonic == "add" and len(b.operands) == 2 \
                        and b.operands[0].type == x86.X86_OP_REG and b.operands[0].reg == self.thunks[t] \
                        and b.operands[1].type == x86.X86_OP_IMM \
                        and (a.address + a.size + b.operands[1].imm) & 0xFFFFFFFF == self.elf.got:
                    got_regs.add(self.thunks[t])
            self.cache[f[0]] = (f, insns, got_regs)
        return self.cache[f[0]]


def find_xrefs_elf(elf, target, branches=True, code=None):
    """ELF32: instructions whose relocated absolute operand (R_386_RELATIVE / R_386_32), GOT-relative
    operand ([REG+disp] with REG = GOT in that function) or, optionally, call/jmp/jcc rel32 (direct or
    R_386_PC32-relocated) resolves to TARGET. Returns (instruction, kind) pairs."""
    code = code or ElfCode(elf)
    text_va, text = elf.text()
    cand = {}
    for va, (t, _, _, _) in elf.relocs.items():
        if not text_va <= va < text_va + len(text) or elf.reloc_value(va) != target:
            continue
        if t in (1, 8):
            cand[va] = "abs"
        elif t == 2 and branches:
            cand[va] = "pc32"
    b = np.frombuffer(text, dtype=np.uint8).astype(np.int64)
    n = len(b) - 3
    d = b[:n] | (b[1:n + 1] << 8) | (b[2:n + 2] << 16) | (b[3:n + 3] << 24)
    d = np.where(d >= 1 << 31, d - (1 << 32), d)
    if branches:
        pos = np.arange(n, dtype=np.int64) + text_va + 4
        for p in np.nonzero(d + pos == target)[0].tolist():
            cand.setdefault(text_va + p, "rel")
    if elf.got is not None:
        for p in np.nonzero(d == target - elf.got)[0].tolist():
            cand.setdefault(text_va + p, "got")
    out = {}
    for a, kind in sorted(cand.items()):
        f, insns, got_regs = code.function(a)
        if f is None:
            continue  # outside every symbol-sized function: padding or unnamed code
        ins = None
        for i in insns:
            if i.address < a < i.address + i.size:
                ins = i
                break
        if ins is None:
            continue
        k = a - ins.address
        if kind == "abs":
            ok = ins.disp_offset == k or ins.imm_offset == k
        elif kind == "pc32":
            ok = ins.imm_offset == k and ins.imm_size == 4 and \
                (capstone.CS_GRP_JUMP in ins.groups or capstone.CS_GRP_CALL in ins.groups)
        elif kind == "rel":
            ok = ins.imm_offset == k and ins.imm_size == 4 and branch_target(ins) == target \
                and a not in elf.relocs
        else:
            ok = ins.disp_offset == k and a not in elf.relocs and any(
                op.type == x86.X86_OP_MEM and op.mem.base in got_regs and op.mem.disp == target - elf.got
                for op in ins.operands)
        if ok:
            out[ins.address] = (ins, kind)
    return [out[a] for a in sorted(out)]


def data_pointers_elf(elf, target):
    """ELF32: relocated dwords outside .text whose loaded value is TARGET (vtables, tables, GOT)."""
    out = []
    for va in sorted(elf.relocs):
        if elf.sec_name(va) != ".text" and elf.relocs[va][0] in (1, 6, 8) and elf.reloc_value(va) == target:
            out.append((va, elf.sec_name(va) or "?"))
    return out


def ins_mask(ins, loose, keep_imm, pe=None):
    m = [False] * ins.size
    def cover(off, size):
        for k in range(off, off + size):
            if 0 <= k < ins.size:
                m[k] = True
    branch = capstone.CS_GRP_JUMP in ins.groups or capstone.CS_GRP_CALL in ins.groups
    if pe is not None and pe.is32:
        r = ins.address - pe.base
        for k in range(ins.size):
            if r + k in pe.fix:
                m[k] = True  # absolute address operand: changes with every rebuild and rebase
        if loose and ins.disp_size:
            cover(ins.disp_offset, ins.disp_size)
    elif rip_target(ins) is not None or rva_disp(ins) is not None:
        cover(ins.disp_offset, 4)  # always disp32; capstone under-reports disp_size for some SSE ops
    elif loose and ins.disp_size:
        cover(ins.disp_offset, ins.disp_size)
    if branch and ins.imm_size and (ins.imm_size == 4 or loose):
        cover(ins.imm_offset, ins.imm_size)
    elif loose and not keep_imm and ins.imm_size:
        cover(ins.imm_offset, ins.imm_size)
    return m


def pattern(seq, site_idx, loose, pe=None):
    raw, mask = b"", []
    for k, ins in enumerate(seq):
        raw += bytes(ins.bytes)
        mask += ins_mask(ins, loose, keep_imm=(k == site_idx), pe=pe)
    return raw, mask


def to_hex(raw, mask):
    return " ".join("??" if w else f"{b:02X}" for b, w in zip(raw, mask))


def to_regex(sig):
    parts = [b"." if t == "??" else re.escape(bytes([int(t, 16)])) for t in sig.split()]
    return re.compile(b"(?=" + b"".join(parts) + b")", re.DOTALL)


def hits(sig, text_va, text):
    return [text_va + m.start() for m in to_regex(sig).finditer(text)]


def make_sig(pe, ins_va, loose):
    insns = pe.insns(ins_va)
    idx = next((k for k, i in enumerate(insns) if i.address == ins_va), None)
    if idx is None:
        raise ValueError(f"{ins_va:#x} is not an instruction start in linear disassembly")
    text_va, text = pe.text()
    lo = hi = idx
    grow_hi = True
    while True:
        seq = insns[lo:hi + 1]
        raw, mask = pattern(seq, idx - lo, loose, pe)
        sig = to_hex(raw, mask)
        h = hits(sig, text_va, text)
        if (len(h) == 1 and len(raw) >= 12) or hi - lo + 1 >= MAX_INSNS or (lo == 0 and hi == len(insns) - 1):
            break
        if (grow_hi and hi + 1 < len(insns)) or lo == 0:
            hi += 1
        else:
            lo -= 1
        grow_hi = not grow_hi
    ins_off = insns[idx].address - insns[lo].address
    return {"sig": sig, "ins_off": ins_off, "hits": len(h), "occurrence": h.index(insns[lo].address)}


def cvar_default(pe, name, reg):
    """Find the ConVar registration that references NAME and return the VA of its default-value
    string: what REG's LEA points to (x64), or the relocated push just before the name push
    (x86 stack arguments, REG = "push")."""
    want = name.encode() + b"\0"
    strs = []
    for sname, va, vs, raw, rs, ch in pe.secs:
        if ch & 0x20000000:
            continue
        j = pe.data.find(want, raw, raw + rs)
        while j != -1:
            if pe.data[j - 1] == 0:
                strs.append(pe.base + va + j - raw)
            j = pe.data.find(want, j + 1, raw + rs)
    found = set()
    if reg == "push":
        for s in strs:
            for ins in find_xrefs32(pe, s, branches=False):
                if ins.mnemonic != "push":
                    continue
                try:
                    run = pe.insns(ins.address)
                except ValueError:
                    continue
                k = next((n for n, i in enumerate(run) if i.address == ins.address), 0)
                if k > 0 and run[k - 1].mnemonic == "push" and abs_target(pe, run[k - 1]) is not None:
                    found.add(abs_target(pe, run[k - 1]))
        return sorted(found)
    text_va, text = pe.text()
    for m in re.finditer(rb"[\x48\x4C]\x8D[\x05\x0D\x15\x1D\x25\x2D\x35\x3D]", text):
        a = text_va + m.start()
        if a + 7 + struct.unpack_from("<i", text, m.start() + 3)[0] not in strs:
            continue
        try:
            insns = pe.insns(a)
        except ValueError:
            continue
        k = next((n for n, i in enumerate(insns) if i.address == a), None)
        if k is None:
            continue
        for i in insns[max(0, k - 6):k + 7]:
            if i.mnemonic == "lea" and i.op_str.startswith(reg + ",") and rip_target(i) is not None:
                found.add(rip_target(i))
    return sorted(found)


def target_set(name):
    """Load reference build identities and patch metadata without a file-patcher dependency."""
    tools = os.path.dirname(os.path.abspath(__file__))
    paths = {
        "gmod": os.path.join(tools, "sites_25375506.json"),
        "gmod-x86": os.path.join(tools, "sites_25375506-x86.json"),
        "vbsppp": os.path.join(tools, "..", "compiler", "sites_vbsppp-80289A94.json"),
    }
    if name not in paths:
        sys.exit(f"unknown target {name!r} (gmod, gmod-x86 or vbsppp)")
    with open(paths[name], encoding="utf-8") as f:
        db = json.load(f)
    sites = [(e["group"], e["module"], (e["id"], int(e["va_gen"], 16), e["len"], e["new"]))
             for e in db["sites"]]
    return db["build"], db["modules"], sites


def data_anchor(pe, pid, va):
    """(anchor, number of code references, counted at) for data site VA. The anchor is the
    recorded REFS instruction if it still references VA, else the first in-.pdata reference with
    a unique strict signature. A site with no exact reference (x86 P5-04: a 1-based index folded
    into the displacement) is anchored on the nearest reference up to 16 bytes below it."""
    at = va
    all_refs = [i.address for i in find_xrefs(pe, va, branches=False)]
    for d in (4, 8, 12, 16):
        if all_refs:
            break
        at = va - d
        all_refs = [i.address for i in find_xrefs(pe, at, branches=False)]
    if not all_refs:
        sys.exit(f"ABORT {pid}: no code reference to data site {va:#x}")
    ref = REFS.get(pid)
    if ref in all_refs:
        return ref, len(all_refs), at
    refs = [r for r in all_refs if pe.func_at(r)] or all_refs
    for r in refs:
        if make_sig(pe, r, False)["hits"] == 1:
            return r, len(all_refs), at
    return refs[0], len(all_refs), at


def gen(root, out, target="gmod"):
    build, mods, site_list = target_set(target)
    cvars = CVARS_X86 if target == "gmod-x86" else CVARS
    pes, sites = {}, []
    for rel, sha in mods.items():
        pe = pes[rel] = PE(os.path.join(root, *rel.split("/")))
        if pe.sha != sha:
            sys.exit(f"ABORT {rel}: hash {pe.sha} is not the reference build")
    for group, rel, (pid, va, length, new) in site_list:
        pe = pes[rel]
        orig_b = pe.read(va, length)
        if orig_b is None or len(orig_b) != length:
            sys.exit(f"ABORT {pid}: reference bytes are not file-backed at {va:#x}")
        e = {"id": pid, "group": group, "module": rel, "va_gen": f"{va:#x}", "len": len(orig_b),
             "new": new}
        if pid in cvars:
            got = cvar_default(pe, *cvars[pid])
            if got != [va]:
                sys.exit(f"ABORT {pid}: cvar anchor gives {[hex(g) for g in got]}, expected {va:#x}")
            e.update(kind="cvar", cvar=cvars[pid][0], reg=cvars[pid][1], orig=orig_b.hex(" ").upper())
            print(f"{pid:7} {rel.split('/')[-1]:11} cvar {cvars[pid][0]} -> {va:#x}")
            sites.append(e)
            continue
        if not pe.is_exec(va):
            ref, nrefs, at = data_anchor(pe, pid, va)
            ins = next(pe.md.disasm(pe.read(ref, 16), ref))
            tgt = ref_target(pe, ins)
            e.update(kind="ref", ref_va_gen=f"{ref:#x}", data_offset=va - tgt, orig=orig_b.hex(" ").upper(),
                     users_gen=nrefs)
            if at != va:
                e["users_at_ref"] = True  # users counted at the referenced address, not the site
            # Content anchor: how many times the exact original data bytes occur in non-executable
            # data. A unique, distinctive run (e.g. the codec numbit tables) is a build-independent
            # fingerprint used as a fallback when the referencing instruction's signature moves.
            e["content_hits"] = len(content_occurrences(pe, orig_b))
            # Reference-build readers, by mnemonic, for changed-user diagnostics at find time.
            e["users_mnem"] = sorted(m for _, m, _ in user_sites(pe, at))
            rfn = pe.func_start(ref)
            e["gen_reader_fn"] = f"{rfn:#x}" if rfn else None  # consumed by the sibling-anchor pass
            site_ins = ref
        else:
            ins = next(i for i in pe.insns(va) if i.address <= va < i.address + i.size)
            tgt = code_ref(pe, ins)
            mask = ins_mask(ins, False, True, pe)
            delta = va - ins.address
            om = [mask[delta + k] if delta + k < len(mask) else False for k in range(len(orig_b))]
            e.update(kind="code", patch_delta=delta, orig=to_hex(orig_b, om))
            if tgt is not None:
                val = pe.read(tgt, 4)
                e["expect_ref"] = val.hex(" ").upper() if val else None
            # Immediate-load sites (e.g. the codec width constants) can be re-found by value inside a
            # content-anchored sibling's function when register reallocation moves the instruction.
            sfn = pe.func_start(ins.address)
            imms = [op for op in ins.operands if op.type == x86.X86_OP_IMM]
            if delta == 0 and len(imms) == 1 and ins.imm_size and isinstance(new, str):
                newb = bytes.fromhex(new)
                e["gen_fn"] = f"{sfn:#x}" if sfn else None
                e["imm_val"] = imms[0].imm
                e["imm_mnem"] = ins.mnemonic
                e["imm_new"] = int.from_bytes(newb[ins.imm_offset:ins.imm_offset + ins.imm_size], "little")
            site_ins = ins.address
        e["strict"] = make_sig(pe, site_ins, False)
        e["loose"] = make_sig(pe, site_ins, True)
        flag = "" if e["strict"]["hits"] == 1 else f"  (strict not unique: {e['strict']['hits']} hits)"
        users = f", {e['users_gen']} user(s)" if "users_gen" in e else ""
        print(f"{pid:7} {rel.split('/')[-1]:11} {e['kind']:4} strict {len(e['strict']['sig'].split()):3}B"
              f" loose hits {e['loose']['hits']}{users}{flag}")
        sites.append(e)
    # Link each immediate-load site to a unique-content data site read by the same function, so a
    # register-moved immediate can be re-found inside that function when its signature fails.
    readers = {}
    for e in sites:
        if e.get("content_hits") == 1 and e.get("gen_reader_fn"):
            readers.setdefault((e["module"], e["gen_reader_fn"]), e["id"])
    for e in sites:
        sib = readers.get((e["module"], e.get("gen_fn"))) if "imm_val" in e else None
        if sib:
            e["fn_sibling"] = sib
        elif "imm_val" in e:
            for k in ("imm_val", "imm_mnem", "imm_new"):
                e.pop(k, None)
        e.pop("gen_fn", None)
        e.pop("gen_reader_fn", None)
    json.dump({"build": build, "modules": mods, "sites": sites}, open(out, "w"), indent=1)
    print(f"WROTE {out} ({len(sites)} sites)")


def content_occurrences(pe, raw):
    """VAs in non-executable initialised data where the exact byte run RAW appears. The content of
    a distinctive data table (e.g. a codec numbit table) is a build-independent fingerprint: unlike
    a referencing instruction it does not move when the compiler reallocates registers."""
    out = []
    for name, va, vs, rawoff, rs, ch in pe.secs:
        if ch & 0x20000000:  # skip executable sections
            continue
        j = pe.data.find(raw, rawoff, rawoff + rs)
        while j != -1:
            out.append(pe.base + va + j - rawoff)
            j = pe.data.find(raw, j + 1, rawoff + rs)
    return out


def user_sites(pe, value):
    """(instruction address, mnemonic, containing function start) for every instruction that reads
    VALUE. Used for the reference-build user fingerprint and changed-user diagnostics."""
    out = []
    for ins in find_xrefs(pe, value, branches=False):
        out.append((ins.address, ins.mnemonic, pe.func_start(ins.address)))
    return out


def candidates(pe, value):
    out = []
    for name, va, vs, raw, rs, ch in pe.secs:
        if ch & 0x80000000 or ch & 0x20000000 or name not in (".rdata", "_RDATA"):
            continue  # read-only, non-executable data only
        j = pe.data.find(value, raw, raw + rs)
        while j != -1:
            rva = va + j - raw
            if not any(rva + b in pe.fix for b in range(4)):
                out.append(pe.base + rva)
            j = pe.data.find(value, j + 1, raw + rs)
    out.sort(key=lambda a: (a % 4 != 0, a))
    return out


# Confidence of a located site:
#   VERIFIED  - strict unique signature, original bytes intact, and (data sites) the same readers.
#               Safe to paste into the patch script.
#   REVIEW    - located and original bytes intact, but by a weaker route (loose-only or
#               occurrence-order signature, unique table content, or with a changed reader count).
#               A human must confirm it before use; these never count as fully approved.
#   MISMATCH  - located but the original bytes differ (the code around it changed).
#   NOT FOUND - no confident match; locate by hand.
VERIFIED, REVIEW, MISMATCH, NOTFOUND = "VERIFIED", "REVIEW", "MISMATCH", "NOT FOUND"


def func_extent(pe, prim):
    """Full address range of the primary function starting at PRIM, spanning any chained .pdata
    fragments (x64). func_at alone returns only the fragment containing a given address."""
    if pe.is32:
        return pe.func_at(prim)
    end = None
    for b, e2, _ in pe.funcs:
        vb = pe.base + b
        if vb < prim:
            continue
        if pe.func_start(vb) == prim:
            end = pe.base + e2
        else:
            break
    return (prim, end) if end else pe.func_at(prim)


def new_entry_str(pes, e, va, orig_now, new_hex=None):
    """The ready-to-paste (id, va, original, new) patch-script tuple for a located site. NEW_HEX,
    when supplied, is the exact replacement bytes (used when a register-moved immediate makes the
    recorded fixed `new` encoding inapplicable)."""
    if new_hex is not None:
        return f'("{e["id"]}", {va:#x}, "{orig_now.hex(" ").upper()}", "{new_hex}"),  # {e["module"]}'
    new = e["new"]
    if isinstance(new, dict):
        if "abs32" in new:
            newv = f'("abs32", {new["abs32"]}, {new["target"]})'
        elif new["target"] == "NEXT_DWORD":
            ins = next(md.disasm(pe_read(pes, e, va), va))
            newv = f'("redirect", {new["redirect"]}, {rip_target(ins) + 4:#x})'
        else:
            newv = f'("redirect", {new["redirect"]}, {new["target"]})'
    else:
        newv = f'"{new}"'
    return f'("{e["id"]}", {va:#x}, "{orig_now.hex(" ").upper()}", {newv}),  # {e["module"]}'


def mnem_delta(ref_mnems, cur_mnems):
    """(added, removed) multisets of reader mnemonics between the reference and the current build."""
    from collections import Counter
    ref, cur = Counter(ref_mnems), Counter(cur_mnems)
    added = sorted((cur - ref).elements())
    removed = sorted((ref - cur).elements())
    return added, removed


def locate_site(pe, e):
    """Locate one non-cvar site. Returns a result dict with status/method/va/orig_now and, for data
    sites, the current readers and a changed-user diagnostic."""
    text_va, text = pe.text()
    found, mode, occ, note_bits = None, None, False, []
    for m in ("strict", "loose"):
        s = e[m]
        h = hits(s["sig"], text_va, text)
        if len(h) == 1 or (len(h) == s["hits"] and s["hits"] > 1):
            found, mode = h[0 if len(h) == 1 else s["occurrence"]] + s["ins_off"], m
            occ = len(h) > 1
            break
    r = {"id": e["id"], "module": e["module"], "group": e["group"], "va_gen": e["va_gen"],
         "status": NOTFOUND, "method": NOTFOUND, "note": "", "va": None, "orig": None,
         "fn": None, "users": None}

    if found is not None:
        ins = next(pe.md.disasm(pe.read(found, 16), found))
        if e["kind"] == "ref":
            tgt = ref_target(pe, ins)
            va = None if tgt is None else tgt + e["data_offset"]
            orig_now = pe.read(va, e["len"]) if va is not None else None
            ok = orig_now is not None and orig_now.hex(" ").upper() == e["orig"]
            read_at = (tgt if e.get("users_at_ref") else va)
        else:
            va = found + e["patch_delta"]
            orig_now = pe.read(va, e["len"])
            want = e["orig"].split()
            ok = orig_now is not None and all(w == "??" or int(w, 16) == b for w, b in zip(want, orig_now))
            if ok and e.get("expect_ref"):
                tgt = code_ref(pe, ins)
                val = pe.read(tgt, 4) if tgt is not None else None
                ok = val is not None and val.hex(" ").upper() == e["expect_ref"]
            read_at = None
        if not ok:
            r.update(status=MISMATCH, method=mode or "?", va=va, orig=orig_now,
                     fn=pe.func_start(found))
            return r
        method = mode
        if mode == "loose":
            note_bits.append("loose signature only")
        if occ:
            method += f" occ {e[mode]['occurrence'] + 1}/{len(hits(e[mode]['sig'], text_va, text))}"
            note_bits.append("resolved by occurrence order (CHECK)")
        status = REVIEW if (mode == "loose" or occ) else VERIFIED
        if e["kind"] == "ref" and "users_gen" in e and read_at is not None:
            us = user_sites(pe, read_at)
            r["users"] = {"count": len(us), "ref_count": e["users_gen"],
                          "sites": [[f"{a:#x}", mn, (f"{fn:#x}" if fn else None)] for a, mn, fn in us]}
            if len(us) != e["users_gen"]:
                status = REVIEW
                note_bits.append(f"reader count {e['users_gen']} -> {len(us)}")
                added, removed = mnem_delta(e.get("users_mnem", []), [mn for _, mn, _ in us])
                if added:
                    note_bits.append("added: " + ", ".join(added))
                if removed:
                    note_bits.append("removed: " + ", ".join(removed))
        r.update(status=status, method=method, va=va, orig=orig_now,
                 fn=pe.func_start(found), note="; ".join(note_bits))
        return r

    # Fallback for data sites: a distinctive data table that occurs exactly once can be located by
    # its own content when the referencing instruction's signature has moved (register reallocation).
    if e["kind"] == "ref" and e.get("content_hits") == 1 and len(e["orig"].split()) >= 8:
        occs = content_occurrences(pe, bytes(int(b, 16) for b in e["orig"].split()))
        if len(occs) == 1:
            va = occs[0]
            orig_now = pe.read(va, e["len"])
            read_at = va - e.get("data_offset", 0) if e.get("users_at_ref") else va
            us = user_sites(pe, read_at)
            if us:  # require the table to still be read by code
                note_bits.append(f"unique table content, {len(us)} reader(s)")
                if "users_gen" in e and len(us) != e["users_gen"]:
                    note_bits.append(f"reader count {e['users_gen']} -> {len(us)}")
                    added, removed = mnem_delta(e.get("users_mnem", []), [mn for _, mn, _ in us])
                    if added:
                        note_bits.append("added: " + ", ".join(added))
                    if removed:
                        note_bits.append("removed: " + ", ".join(removed))
                r.update(status=REVIEW, method="content", va=va, orig=orig_now,
                         fn=us[0][2], note="; ".join(note_bits),
                         users={"count": len(us), "ref_count": e.get("users_gen"),
                                "sites": [[f"{a:#x}", mn, (f"{fn:#x}" if fn else None)] for a, mn, fn in us]})
    return r


def find(root, sites_path, json_out=None):
    db = json.load(open(sites_path))
    print(f"Signatures from build {db['build']}")
    pes = {}
    for rel, sha in db["modules"].items():
        pe = pes[rel] = PE(os.path.join(root, *rel.split("/")))
        same = "IDENTICAL to analysed build" if pe.sha == sha else "different build"
        print(f"{rel}: {pe.sha}  ({same})")
    print()
    results = []
    for e in db["sites"]:
        pe = pes[e["module"]]
        if e["kind"] == "cvar":
            got = cvar_default(pe, e["cvar"], e["reg"])
            va = got[0] if len(got) == 1 else None
            orig_now = pe.read(va, e["len"]) if va else None
            if orig_now is not None and orig_now.hex(" ").upper() == e["orig"]:
                status = VERIFIED if len(got) == 1 else REVIEW
            else:
                status = MISMATCH if va else NOTFOUND
            r = {"id": e["id"], "module": e["module"], "group": e["group"], "va_gen": e["va_gen"],
                 "status": status, "method": f"cvar '{e['cvar']}' ({len(got)} reg)", "va": va,
                 "orig": orig_now, "fn": None, "users": None, "new_bytes_hex": None,
                 "note": "" if len(got) == 1 else "multiple registrations"}
        else:
            r = locate_site(pe, e)
            r["new_bytes_hex"] = None
        results.append((e, r))

    # Sibling-scoped immediate fallback: an immediate-load site that no signature matched can be
    # recovered inside the function of a content-anchored sibling (register reallocation only moved
    # the register, not the constant). The spliced replacement bytes carry the new register.
    rmap = {e["id"]: (e, r) for e, r in results}
    for e, r in results:
        if r["status"] != NOTFOUND or "fn_sibling" not in e or "imm_val" not in e:
            continue
        sib = rmap.get(e["fn_sibling"])
        if not sib or sib[1]["fn"] is None or sib[1]["status"] not in (VERIFIED, REVIEW):
            continue
        pe = pes[e["module"]]
        rng = func_extent(pe, sib[1]["fn"])
        if not rng:
            continue
        cand = [ins for ins in pe.md.disasm(pe.read(rng[0], rng[1] - rng[0]), rng[0])
                if ins.mnemonic == e["imm_mnem"] and ins.imm_size
                and any(op.type == x86.X86_OP_IMM and op.imm == e["imm_val"] for op in ins.operands)]
        if len(cand) != 1:
            continue
        ins = cand[0]
        nb = bytearray(ins.bytes)
        nb[ins.imm_offset:ins.imm_offset + ins.imm_size] = int(e["imm_new"]).to_bytes(ins.imm_size, "little")
        r.update(status=REVIEW, method="sibling fn", va=ins.address, orig=bytes(ins.bytes),
                 fn=sib[1]["fn"], new_bytes_hex=bytes(nb).hex(" ").upper(),
                 note=f"register-moved immediate in {e['fn_sibling']}'s function "
                      f"(imm {e['imm_val']:#x} -> {e['imm_new']:#x})")

    for e, r in results:
        r["new_entry"] = (new_entry_str(pes, e, r["va"], r["orig"], r.get("new_bytes_hex"))
                          if r["status"] in (VERIFIED, REVIEW) and r["va"] is not None else None)
        moved = f"{int(e['va_gen'], 16):#x} -> {r['va']:#x}" if r["va"] else e["va_gen"] + " -> ?"
        fn = f" in fn {r['fn']:#x}" if r["fn"] else ""
        note = f"  [{r['note']}]" if r["note"] else ""
        print(f"{r['status']:9} {e['id']:7} {e['module'].split('/')[-1]:11} {moved:30} "
              f"via {r['method']}{fn}{note}")
        if r["users"] and r["status"] == REVIEW and r["users"].get("ref_count") not in (None, r["users"]["count"]):
            for a, mn, ufn in r["users"]["sites"]:
                print(f"              reader {a} {mn:8} fn {ufn}")

    nver = sum(1 for _, r in results if r["status"] == VERIFIED)
    nrev = sum(1 for _, r in results if r["status"] == REVIEW)
    nfail = sum(1 for _, r in results if r["status"] in (MISMATCH, NOTFOUND))

    print("\n# Proposed patch-script entries - VERIFIED (strict unique match, safe to paste).")
    for group in dict.fromkeys(e["group"] for e in db["sites"]):
        shown = [r for e, r in results if r["group"] == group and r["status"] == VERIFIED]
        if shown:
            print(f"# ---- {group}")
            for r in shown:
                print(f"    {r['new_entry']}")

    if nrev:
        print("\n# ==== REVIEW REQUIRED - located but NOT verified. Confirm each (tools/README.md),"
              " then uncomment to use. Never paste these as-is.")
        for e, r in results:
            if r["status"] == REVIEW:
                print(f"#   {r['id']}: {r['note'] or r['method']}")
                if r["new_entry"]:
                    print(f"#     {r['new_entry']}")
    if nfail:
        print("\n# ==== NOT RESOLVED - locate by hand (tools/README.md).")
        for e, r in results:
            if r["status"] in (MISMATCH, NOTFOUND):
                print(f"#   {r['id']}: {r['status']} ({r['module']})")

    if any(isinstance(e["new"], dict) for e in db["sites"]):
        print("\n# Redirect-target candidates: reloc-free +-32768.0f in read-only data (aligned first).")
        print("# Pick one per module and sign; confirm it is not inside a relocated pointer.")
        for rel, pe in pes.items():
            for name, val in (("POS32768", b"\x00\x00\x00\x47"), ("NEG32768", b"\x00\x00\x00\xC7")):
                c = candidates(pe, val)
                shown = ", ".join(f"{a:#x}{'' if a % 4 == 0 else ' (unaligned)'}" for a in c[:4])
                print(f"#   {rel.split('/')[-1]:11} {name}: {shown or 'NONE'}")

    print(f"\n{nver} verified, {nrev} need review, {nfail} not resolved / {len(results)} sites")
    if json_out:
        report = {"build": db["build"], "root": root,
                  "modules": {rel: pe.sha for rel, pe in pes.items()},
                  "summary": {"verified": nver, "review": nrev, "failed": nfail, "total": len(results)},
                  "sites": [{"id": r["id"], "module": r["module"], "group": r["group"],
                             "status": r["status"], "method": r["method"], "note": r["note"],
                             "va_gen": r["va_gen"], "va": (f"{r['va']:#x}" if r["va"] else None),
                             "fn": (f"{r['fn']:#x}" if r["fn"] else None),
                             "orig": (r["orig"].hex(" ").upper() if r["orig"] else None),
                             "users": r["users"], "proposed_entry": r["new_entry"]}
                            for _, r in results]}
        json.dump(report, open(json_out, "w"), indent=1)
        print(f"WROTE {json_out}")
    # Exit 0 only if every site is VERIFIED; 1 if any hard failure; 2 if only reviews remain.
    return 1 if nfail else (2 if nrev else 0)


def describe(elf, va):
    s = elf.sym_at(va)
    return f"{s[0]}+{s[1]:#x}" if s else "?"


def xrefs(path, va, data=False):
    raw = open(path, "rb").read(4)
    if is_elf(raw):
        elf = ElfImage(open(path, "rb").read(), path)
        refs = find_xrefs_elf(elf, va)
        for ins, kind in refs:
            f = elf.func_at(ins.address)
            ops = ins.op_str if kind != "pc32" else f"{va:#x}  (relocated R_386_PC32; file holds a placeholder)"
            print(f"{ins.address:#x}  {kind:4}  fn {f[0]:#x} {f[2]}  |  {ins.mnemonic} {ops}")
        print(f"{len(refs)} reference(s) to {va:#x} ({describe(elf, va)})")
        if data:
            ptrs = data_pointers_elf(elf, va)
            for a, sec in ptrs:
                print(f"{a:#x}  data pointer in {sec} ({describe(elf, a)})")
            print(f"{len(ptrs)} relocated data pointer(s) to {va:#x}")
        return
    pe = PE(path)
    refs = find_xrefs(pe, va)
    for ins in refs:
        fn = pe.func_start(ins.address)
        fn = f"fn {fn:#x}" if fn else "fn ? (no .pdata)"
        print(f"{ins.address:#x}  {fn:22} {ins.mnemonic} {ins.op_str}")
    print(f"{len(refs)} reference(s) to {va:#x}")
    if data:
        ptrs = data_pointers(pe, va)
        for a, sec in ptrs:
            print(f"{a:#x}  data pointer in {sec}")
        print(f"{len(ptrs)} relocated data pointer(s) to {va:#x}")


def sym(path, query):
    """ELF: symbols whose (mangled) name contains QUERY, or the symbol covering a hex VA."""
    elf = ElfImage(open(path, "rb").read(), path)
    if re.fullmatch(r"0x[0-9a-fA-F]+", query):
        va = int(query, 16)
        print(f"{va:#x}  {describe(elf, va)}  [{elf.sec_name(va) or 'no section'}]")
        return
    found = elf.find_symbols(query)
    for v, size, name, t in found:
        print(f"{v:#010x}  {size:#8x}  {t[4:]:6}  {name}")
    print(f"{len(found)} symbol(s)")


def pe_read(pes, e, va):
    return pes[e["module"]].read(va, 16)


if __name__ == "__main__":
    a = sys.argv[1:]
    if len(a) in (3, 5) and a[0] == "gen" and (len(a) == 3 or a[3] == "--target"):
        gen(a[1], a[2], a[4] if len(a) == 5 else "gmod")
    elif len(a) in (3, 5) and a[0] == "find" and (len(a) == 3 or a[3] == "--json"):
        sys.exit(find(a[1], a[2], a[4] if len(a) == 5 else None))
    elif len(a) in (3, 4) and a[0] == "xrefs" and (len(a) == 3 or a[3] == "--data"):
        xrefs(a[1], int(a[2], 16), len(a) == 4)
    elif len(a) == 3 and a[0] == "sym":
        sym(a[1], a[2])
    else:
        sys.exit(__doc__)
