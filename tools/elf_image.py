"""Read-only view of a 32-bit x86 ELF shared object (Linux srcds: engine_srv.so, server_srv.so, ...).

Exposes section and relocation metadata for the site tools and codec emulator:
  base       0 (link-time addresses are offsets from the load base)
  secs       (va, mapped size, file offset, file size, characteristics) for allocated sections, with
             PE-style characteristics: 0x20000000 exec, 0x40000000 read, 0x80000000 write
  fixstarts  va -> (kind, 4) for every 4-byte dynamic relocation. R_386_RELATIVE is reported as kind 3
             (same as a PE HIGHLOW: in-file value + load base), every other type by name, e.g.
             "R_386_32", so code that accepts only (3, 4) rejects values the loader computes otherwise.
  fix        every byte covered by a relocation

Valve's Linux libraries are mostly non-PIC with text relocations (DT_TEXTREL), so absolute operands in
.text carry R_386_RELATIVE or R_386_32 relocations; calls to exported functions may carry R_386_PC32.
Some code is PIC and addresses data relative to the GOT (_GLOBAL_OFFSET_TABLE_, start of .got.plt).
"""
import bisect
import hashlib
import struct

from elftools.elf.elffile import ELFFile
from elftools.elf.relocation import RelocationSection

R_386_32, R_386_PC32, R_386_GLOB_DAT, R_386_JMP_SLOT, R_386_RELATIVE = 1, 2, 6, 7, 8
RELOC_NAMES = {R_386_32: "R_386_32", R_386_PC32: "R_386_PC32", R_386_GLOB_DAT: "R_386_GLOB_DAT",
               R_386_JMP_SLOT: "R_386_JMP_SLOT", R_386_RELATIVE: "R_386_RELATIVE"}
SHF_WRITE, SHF_ALLOC, SHF_EXECINSTR = 1, 2, 4


def is_elf(data):
    return data[:4] == b"\x7fELF"


class ElfImage:
    def __init__(self, data, path=None):
        import io
        self.path = path
        self.data = data
        self.sha = hashlib.sha256(data).hexdigest().upper()
        elf = ELFFile(io.BytesIO(data))
        if elf.elfclass != 32 or elf.header.e_machine != "EM_386":
            raise ValueError(f"{path or 'image'}: only 32-bit x86 ELF is supported")
        self.is32 = True
        self.is_elf = True
        self.base = 0
        self.secs, self.sec_names = [], []
        for s in elf.iter_sections():
            fl = s["sh_flags"]
            if not fl & SHF_ALLOC or not s["sh_size"]:
                continue
            ch = 0x40000000 | (0x20000000 if fl & SHF_EXECINSTR else 0) | (0x80000000 if fl & SHF_WRITE else 0)
            fsize = 0 if s["sh_type"] == "SHT_NOBITS" else s["sh_size"]
            self.secs.append((s["sh_addr"], s["sh_size"], s["sh_offset"], fsize, ch))
            self.sec_names.append(s.name)

        self.symbols = []  # (value, size, name, type, defined)
        seen = set()
        dynsyms = []
        for tab in (".symtab", ".dynsym"):
            st = elf.get_section_by_name(tab)
            if st is None:
                continue
            for sym in st.iter_symbols():
                if tab == ".dynsym":
                    dynsyms.append(sym)
                t = sym["st_info"]["type"]
                if not sym.name or t not in ("STT_FUNC", "STT_OBJECT", "STT_NOTYPE"):
                    continue
                defined = sym["st_shndx"] != "SHN_UNDEF"
                key = (sym["st_value"], sym.name)
                if key in seen:
                    continue
                seen.add(key)
                self.symbols.append((sym["st_value"], sym["st_size"], sym.name, t, defined))
        self.got = next((v for v, _, n, _, d in self.symbols if n == "_GLOBAL_OFFSET_TABLE_" and d), None)
        if self.got is None:
            gp = elf.get_section_by_name(".got.plt")
            self.got = gp["sh_addr"] if gp is not None else None

        funcs = {}
        for v, size, name, t, d in self.symbols:
            if t == "STT_FUNC" and d and size and self.is_exec(v):
                funcs.setdefault(v, (v, v + size, name))
        self.funcs = sorted(funcs.values())
        self.fbegins = [f[0] for f in self.funcs]

        # va -> (type, symbol name or None, symbol value or None (undefined), in-file addend)
        self.relocs = {}
        self.fixstarts, self.fix = {}, set()
        for s in elf.iter_sections():
            if not isinstance(s, RelocationSection) or s.is_RELA():
                continue
            for r in s.iter_relocations():
                t = r["r_info_type"]
                if t not in RELOC_NAMES:
                    continue
                va = r["r_offset"]
                name = value = None
                if r["r_info_sym"]:
                    sym = dynsyms[r["r_info_sym"]]
                    name = sym.name
                    if sym["st_shndx"] != "SHN_UNDEF":
                        value = sym["st_value"]
                addend = self.dword(va)
                self.relocs[va] = (t, name, value, 0 if addend is None else addend)  # NOBITS: memory starts at 0
                self.fixstarts[va] = (3 if t == R_386_RELATIVE else RELOC_NAMES[t], 4)
                self.fix.update(range(va, va + 4))

    def sec(self, va):
        for s in self.secs:
            if s[0] <= va < s[0] + s[1]:
                return s
        raise ValueError(f"{va:#x} not in any section")

    def sec_name(self, va):
        for s, n in zip(self.secs, self.sec_names):
            if s[0] <= va < s[0] + s[1]:
                return n
        return None

    def off(self, va):
        sva, size, raw, rsize, _ = self.sec(va)
        if va - sva >= rsize:
            raise ValueError(f"{va:#x} is not file-backed")
        return raw + va - sva

    def overlaps_fix(self, va, n):
        return any(va + k in self.fix for k in range(n))

    def is_exec(self, va):
        try:
            return bool(self.sec(va)[4] & 0x20000000)
        except ValueError:
            return False

    def read(self, va, n):
        try:
            o = self.off(va)
        except ValueError:
            return None
        return self.data[o:o + n]

    def dword(self, va):
        b = self.read(va, 4)
        return None if b is None or len(b) < 4 else struct.unpack("<I", b)[0]

    def text(self):
        i = self.sec_names.index(".text")
        va, _, raw, rsize, _ = self.secs[i]
        return va, self.data[raw:raw + rsize]

    def reloc_value(self, va):
        """Link-time value the loader stores at relocated dword VA (target address for PC32), or None
        when it depends on another library (undefined symbol)."""
        r = self.relocs.get(va)
        if r is None:
            return None
        t, _, s, a = r
        if t == R_386_RELATIVE:
            return a
        if s is None:
            return None
        if t == R_386_32:
            return (s + a) & 0xFFFFFFFF
        if t == R_386_PC32:
            return (s + a + 4) & 0xFFFFFFFF  # rel32 operand: target = S + A - P + (P + 4)
        return s  # GLOB_DAT / JMP_SLOT

    def func_at(self, va):
        i = bisect.bisect_right(self.fbegins, va) - 1
        if i >= 0 and self.funcs[i][0] <= va < self.funcs[i][1]:
            return self.funcs[i]
        return None

    def sym_at(self, va):
        """Name and offset of the defined symbol covering VA (functions first), or None."""
        f = self.func_at(va)
        if f:
            return f[2], va - f[0]
        best = None
        for v, size, name, _, d in self.symbols:
            if d and v <= va < v + max(size, 1) and (best is None or v > best[0]):
                best = (v, name)
        return (best[1], va - best[0]) if best else None

    def find_symbols(self, needle):
        n = needle.lower()
        return sorted((v, size, name, t) for v, size, name, t, d in self.symbols if d and n in name.lower())
