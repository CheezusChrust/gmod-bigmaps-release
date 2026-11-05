"""Inspect coordinate ranges in a Source BSP (v19-v21) and optionally compare two BSPs by lump.

Reports world-model bounds, vertex range, plane-distance range, and node/leaf int16 boxes,
flagging any box with mins > maxs (the signature of a wrapped 16-bit store).

Usage: python compiler/inspect_bsp.py MAP.bsp [OTHER.bsp]
"""
import struct
import sys

LUMP_NAMES = {0: "entities", 1: "planes", 2: "texdata", 3: "vertexes", 4: "visibility", 5: "nodes",
              6: "texinfo", 7: "faces", 10: "leafs", 12: "edges", 13: "surfedges", 14: "models",
              16: "leaffaces", 17: "leafbrushes", 18: "brushes", 19: "brushsides", 20: "areas",
              29: "physcollide", 35: "game lump", 40: "pakfile", 43: "texdata strings"}


def lumps(data):
    ident, version = struct.unpack_from("<4si", data, 0)
    if ident != b"VBSP":
        sys.exit("not a VBSP file")
    out = []
    for i in range(64):
        off, length, ver, _ = struct.unpack_from("<iiii", data, 8 + 16 * i)
        out.append((off, length, ver))
    return version, out


def boxes(data, lump, stride, box_off):
    off, length, _ = lump
    bad, lo, hi = 0, [32767] * 3, [-32768] * 3
    for i in range(length // stride):
        mins = struct.unpack_from("<3h", data, off + i * stride + box_off)
        maxs = struct.unpack_from("<3h", data, off + i * stride + box_off + 6)
        if any(mins[k] > maxs[k] for k in range(3)):
            bad += 1
        lo = [min(lo[k], mins[k]) for k in range(3)]
        hi = [max(hi[k], maxs[k]) for k in range(3)]
    return length // stride, bad, lo, hi


def report(path):
    data = open(path, "rb").read()
    version, ls = lumps(data)
    print(f"{path}: BSP v{version}, {len(data)} bytes")
    off, length, _ = ls[14]
    mins, maxs = struct.unpack_from("<3f", data, off), struct.unpack_from("<3f", data, off + 12)
    print(f"  world model mins {mins} maxs {maxs}")
    off, length, _ = ls[3]
    v = [struct.unpack_from("<3f", data, off + 12 * i) for i in range(length // 12)]
    print(f"  vertexes {len(v)}: min {[min(p[k] for p in v) for k in range(3)]} "
          f"max {[max(p[k] for p in v) for k in range(3)]}")
    off, length, _ = ls[1]
    d = [struct.unpack_from("<f", data, off + 20 * i + 12)[0] for i in range(length // 20)]
    print(f"  planes {len(d)}: dist range {min(d)} .. {max(d)}")
    n, bad, lo, hi = boxes(data, ls[5], 32, 12)
    print(f"  nodes {n}: inverted boxes {bad}, overall mins {lo} maxs {hi}")
    leaf_stride = 56 if ls[10][2] == 0 else 32
    n, bad, lo, hi = boxes(data, ls[10], leaf_stride, 8)
    print(f"  leafs {n} (lump v{ls[10][2]}, {leaf_stride} B): inverted boxes {bad}, overall mins {lo} maxs {hi}")
    return data, ls


def compare(a, b):
    (da, la), (db, lb) = a, b
    same, diff = [], []
    for i in range(64):
        ba = da[la[i][0]:la[i][0] + la[i][1]]
        bb = db[lb[i][0]:lb[i][0] + lb[i][1]]
        if not ba and not bb:
            continue
        (same if ba == bb else diff).append(LUMP_NAMES.get(i, str(i)))
    print(f"  identical lumps: {', '.join(same)}")
    print(f"  differing lumps: {', '.join(diff) or 'none'}")


if __name__ == "__main__":
    if len(sys.argv) not in (2, 3):
        sys.exit(__doc__)
    first = report(sys.argv[1])
    if len(sys.argv) == 3:
        compare(first, report(sys.argv[2]))
