"""Read-only PE helper: find byte patterns and convert VA/RVA/file offsets.

Usage:
  python tools/pe_find.py FILE info
  python tools/pe_find.py FILE find HEXBYTES [SECTION]   e.g. 00008046 .rdata
  python tools/pe_find.py FILE va2off VA                 e.g. 0x1802359e0
  python tools/pe_find.py FILE read VA LENGTH
Never writes to FILE.
"""
import hashlib
import struct
import sys


def load(path):
    with open(path, "rb") as f:
        data = f.read()
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    machine, nsec = struct.unpack_from("<HH", data, pe + 4)
    opt_size = struct.unpack_from("<H", data, pe + 20)[0]
    opt = pe + 24
    magic = struct.unpack_from("<H", data, opt)[0]
    base = struct.unpack_from("<Q", data, opt + 24)[0] if magic == 0x20B else struct.unpack_from("<I", data, opt + 28)[0]
    secs = []
    for i in range(nsec):
        o = opt + opt_size + 40 * i
        name = data[o:o + 8].rstrip(b"\0").decode()
        vsize, va, rsize, raw = struct.unpack_from("<IIII", data, o + 8)
        secs.append((name, va, vsize, raw, rsize))
    return data, machine, base, secs


def va2off(base, secs, va):
    rva = va - base
    for name, sva, vsize, raw, rsize in secs:
        if sva <= rva < sva + max(vsize, rsize):
            return name, rva, raw + (rva - sva) if rva - sva < rsize else None
    return None, rva, None


def main():
    path, cmd = sys.argv[1], sys.argv[2]
    data, machine, base, secs = load(path)
    if cmd == "info":
        print(f"sha256={hashlib.sha256(data).hexdigest().upper()} machine=0x{machine:04X} base=0x{base:X}")
        for s in secs:
            print("%-8s rva=0x%08X vsize=0x%08X raw=0x%08X rsize=0x%08X" % s)
    elif cmd == "find":
        pat = bytes.fromhex(sys.argv[3])
        only = sys.argv[4] if len(sys.argv) > 4 else None
        for name, sva, vsize, raw, rsize in secs:
            if only and name != only:
                continue
            blob = data[raw:raw + rsize]
            i = blob.find(pat)
            while i != -1:
                print(f"{name} va=0x{base + sva + i:X} rva=0x{sva + i:X} off=0x{raw + i:X}")
                i = blob.find(pat, i + 1)
    elif cmd == "va2off":
        name, rva, off = va2off(base, secs, int(sys.argv[3], 16))
        print(f"section={name} rva=0x{rva:X} off={'0x%X' % off if off is not None else 'n/a'}")
    elif cmd == "read":
        name, rva, off = va2off(base, secs, int(sys.argv[3], 16))
        print(data[off:off + int(sys.argv[4], 0)].hex(" ").upper())


if __name__ == "__main__":
    main()
