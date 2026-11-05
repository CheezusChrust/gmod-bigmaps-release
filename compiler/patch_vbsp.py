"""Build a verified patched copy of the supported vbsp++ executable."""
import argparse
import hashlib
import os
import struct
import sys

SHA256 = "80289A94437E6A8CB96F74D14B0EF86ED78A28465A4862241D02F2816C120D92"
BASE = 0x140000000

F_POS, F_NEG = "00 00 00 47", "00 00 00 C7"          # +-32768.0f
F_POS_16384, F_NEG_16384 = "00 00 80 46", "00 00 80 C6"
BOX_NEG = "C7 84 24 88 00 00 00 00 00 80 C6"         # mov [rsp+0x88], -16384.0f
BOX_POS = "C7 84 24 A8 0D 00 00 00 00 80 46"         # mov [rsp+0xda8], +16384.0f
MOV_EAX_8000, MOV_EAX_10000 = "B8 00 80 00 00", "B8 00 00 01 00"

# (id, va, original hex, replacement hex). Data entries are .rdata literals whose every
# reference (pdata walk + brute-force disp32 scan of .text) is a world-bounds site.
PATCHES = [
    # Node/leaf bounds: maxs[1..2] were truncated to int16 (pshuflw); saturate like mins (packssdw).
    ("P5-10a", 0x1401E520D, "F2 0F 70 C0 E8", "66 0F 6B C0 90"),   # EmitLeaf
    ("P5-10b", 0x1401E5D51, "F2 0F 70 C0 E8", "66 0F 6B C0 90"),   # EmitDrawNode_r
    # World-bound scalars: brush range, bogus-brush, map-bounds filter, map-border check, water sentinel.
    ("P5-11a", 0x14027AD98, F_POS_16384, F_POS),
    ("P5-11b", 0x14027AD9C, F_NEG_16384, F_NEG),
    # World boxes passed to MakeBspBrushList/BrushBSP (x/y pairs + z immediates).
    ("P5-12a", 0x14027C030, F_NEG_16384 + " " + F_NEG_16384, F_NEG + " " + F_NEG),
    ("P5-12b", 0x14027C040, F_POS_16384 + " " + F_POS_16384, F_POS + " " + F_POS),
    ("P5-12c", 0x1401C0A71, BOX_NEG, BOX_NEG[:-11] + F_NEG),
    ("P5-12d", 0x1401C0A8D, "C7 44 24 78 " + F_POS_16384, "C7 44 24 78 " + F_POS),
    ("P5-12e", 0x1401DCD8E, "C7 44 24 48 " + F_NEG_16384, "C7 44 24 48 " + F_NEG),
    ("P5-12f", 0x1401DCDAB, "C7 44 24 38 " + F_POS_16384, "C7 44 24 38 " + F_POS),
    ("P5-12g", 0x1401E1130, BOX_NEG, BOX_NEG[:-11] + F_NEG),
    ("P5-12h", 0x1401E114C, BOX_POS, BOX_POS[:-11] + F_POS),
    ("P5-12i", 0x1401E1C6E, BOX_NEG, BOX_NEG[:-11] + F_NEG),
    ("P5-12j", 0x1401E1C83, BOX_POS, BOX_POS[:-11] + F_POS),
    ("P5-12k", 0x1401E21D2, BOX_NEG, BOX_NEG[:-11] + F_NEG),
    ("P5-12l", 0x1401E21F5, BOX_POS, BOX_POS[:-11] + F_POS),
    # Water leaf minZ sentinel.
    ("P5-13", 0x1401B77F9, "C7 44 06 10 " + F_POS_16384, "C7 44 06 10 " + F_POS),
    # MakeTreePortals_r unbounded-volume limit: MAX_COORD + SIDESPACE(8) = +-16392 -> +-32776.
    ("P5-14a", 0x14027D2F8, "00 10 80 C6", "00 08 00 C7"),
    ("P5-14b", 0x14027D2FC, "00 10 80 46", "00 08 00 47"),
    # Block grid: BLOCKS_SPACE = COORD_EXTENT / blocksize; grid allocation follows automatically.
    ("P5-15a", 0x1401DCAEA, MOV_EAX_8000, MOV_EAX_10000),       # BlockTree
    ("P5-15b", 0x1401DCE70, MOV_EAX_8000, MOV_EAX_10000),       # ProcessBlock_Thread
    ("P5-15c", 0x1401E2016, MOV_EAX_8000, MOV_EAX_10000),       # ProcessWorldModel clamp
    ("P5-15d", 0x1401E2124, MOV_EAX_8000, MOV_EAX_10000),       # ProcessWorldModel store
    ("P5-15e", 0x1401E3664, MOV_EAX_8000, MOV_EAX_10000),       # block limits init
    # Vertex hash: (v + 16384) >> 7 -> (v + 32768) >> 8; HASH_SIZE stays 256 (table/stride unchanged).
    ("P5-16a", 0x14027B940, "00 40 00 00 00 40 00 00", "00 80 00 00 00 80 00 00"),  # HashVec addend
    ("P5-16b", 0x1401B1F5C, "66 0F 72 E3 07", "66 0F 72 E3 08"),                    # HashVec psrad
    ("P5-16c", 0x1401B2E09, "05 00 40 00 00", "05 00 80 00 00"),                    # FindEdgeVerts
    ("P5-16d", 0x1401B2E17, "C1 F8 07", "C1 F8 08"),
    ("P5-16e", 0x1401B2E24, "81 C1 00 40 00 00", "81 C1 00 80 00 00"),
    ("P5-16f", 0x1401B2E2A, "C1 F9 07", "C1 F9 08"),
    ("P5-16g", 0x1401B2E40, "81 C2 00 40 00 00", "81 C2 00 80 00 00"),
    ("P5-16h", 0x1401B2E46, "C1 FA 07", "C1 FA 08"),
    ("P5-16i", 0x1401B2E5C, "41 81 C2 00 40 00 00", "41 81 C2 00 80 00 00"),
    ("P5-16j", 0x1401B2E63, "41 C1 FA 07", "41 C1 FA 08"),
]


def file_offset(data, va):
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    count = struct.unpack_from("<H", data, pe + 6)[0]
    first = pe + 24 + struct.unpack_from("<H", data, pe + 20)[0]
    rva = va - BASE
    for i in range(count):
        vsize, sva, rsize, raw = struct.unpack_from("<IIII", data, first + 40 * i + 8)
        if sva <= rva < sva + max(vsize, rsize):
            if rva - sva >= rsize:
                raise ValueError(f"{va:#x} is not file-backed")
            return raw + rva - sva
    raise ValueError(f"{va:#x} not in any section")


def build(source, destination):
    with open(source, "rb") as f:
        data = bytearray(f.read())
    digest = hashlib.sha256(data).hexdigest().upper()
    if digest != SHA256:
        sys.exit(f"ABORT {source}: input hash {digest} != expected {SHA256}")
    for pid, va, orig, new in PATCHES:
        orig_b, new_b = bytes.fromhex(orig), bytes.fromhex(new)
        off = file_offset(data, va)
        if bytes(data[off:off + len(orig_b)]) != orig_b:
            sys.exit(f"ABORT {pid}: original bytes differ at {va:#x}")
        if len(new_b) != len(orig_b):
            sys.exit(f"ABORT {pid}: length mismatch")
        data[off:off + len(new_b)] = new_b
        print(f"{pid} va={va:#x} off={off:#x} {orig_b.hex(' ').upper()} -> {new_b.hex(' ').upper()}")
    os.makedirs(os.path.dirname(os.path.abspath(destination)), exist_ok=True)
    with open(destination, "xb") as f:
        f.write(data)
    print(f"WROTE {destination} sha256={hashlib.sha256(data).hexdigest().upper()}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", help="path to a copy of the original vbsp++ executable")
    parser.add_argument("destination", help="new output file; existing files are not overwritten")
    args = parser.parse_args()
    build(args.source, args.destination)
