"""Write a controlled oversized test map (VMF) for compiler tests.

A sealed room whose interior spans +-HALF units in X/Y (default 31744, beyond the stock +-16384
limit), with pillars near the old boundary and the far corners, a player start near one corner
and a light_environment. Uses stock tool/dev materials only.

Usage: python compiler/make_test_vmf.py OUT.vmf [HALF]
"""
import sys

WALL = 64
HEIGHT = 2048
FLOOR_MAT = "DEV/DEV_MEASUREGENERIC01B"
PILLAR_MAT = "DEV/DEV_MEASUREWALL01A"
SKY_MAT = "TOOLS/TOOLSSKYBOX"


class Ids:
    n = 0

    def __call__(self):
        Ids.n += 1
        return Ids.n


nid = Ids()


def side(p, mat, scale):
    (a, b, c) = p
    pts = " ".join("(%g %g %g)" % q for q in (a, b, c))
    ux, vx = ("[1 0 0 0] 0.25", "[0 -1 0 0] 0.25") if p[0][2] == p[1][2] == p[2][2] else \
        (("[0 1 0 0] 0.25", "[0 0 -1 0] 0.25") if p[0][0] == p[1][0] == p[2][0] else
         ("[1 0 0 0] 0.25", "[0 0 -1 0] 0.25"))
    return (f'\t\tside\n\t\t{{\n\t\t\t"id" "{nid()}"\n\t\t\t"plane" "{pts}"\n\t\t\t"material" "{mat}"\n'
            f'\t\t\t"uaxis" "{ux}"\n\t\t\t"vaxis" "{vx}"\n\t\t\t"rotation" "0"\n'
            f'\t\t\t"lightmapscale" "{scale}"\n\t\t\t"smoothing_groups" "0"\n\t\t}}\n')


def outward(face, center):
    """Order points so (p0 - p1) x (p2 - p1) points away from the box center (SDK PlaneFromPoints)."""
    p0, p1, p2 = face
    t1 = [p0[i] - p1[i] for i in range(3)]
    t2 = [p2[i] - p1[i] for i in range(3)]
    n = (t1[1] * t2[2] - t1[2] * t2[1], t1[2] * t2[0] - t1[0] * t2[2], t1[0] * t2[1] - t1[1] * t2[0])
    if sum(n[i] * (p1[i] - center[i]) for i in range(3)) < 0:
        return (p2, p1, p0)
    return face


def box(x1, y1, z1, x2, y2, z2, mat, scale=16):
    faces = [
        ((x1, y2, z2), (x2, y2, z2), (x2, y1, z2)),  # top
        ((x1, y1, z1), (x2, y1, z1), (x2, y2, z1)),  # bottom
        ((x1, y2, z2), (x1, y1, z2), (x1, y1, z1)),  # -x
        ((x2, y2, z1), (x2, y1, z1), (x2, y1, z2)),  # +x
        ((x2, y2, z2), (x1, y2, z2), (x1, y2, z1)),  # +y
        ((x2, y1, z1), (x1, y1, z1), (x1, y1, z2)),  # -y
    ]
    c = ((x1 + x2) / 2, (y1 + y2) / 2, (z1 + z2) / 2)
    faces = [outward(f, c) for f in faces]
    return f'\tsolid\n\t{{\n\t\t"id" "{nid()}"\n' + "".join(side(f, mat, scale) for f in faces) + "\t}\n"


def entity(cls, origin, **kv):
    body = "".join(f'\t"{k}" "{v}"\n' for k, v in kv.items())
    return (f'entity\n{{\n\t"id" "{nid()}"\n\t"classname" "{cls}"\n{body}'
            f'\t"origin" "{origin[0]} {origin[1]} {origin[2]}"\n}}\n')


def write(path, half):
    o = half + WALL
    solids = [
        box(-o, -o, -WALL, o, o, 0, FLOOR_MAT, 64),           # floor
        box(-o, -o, HEIGHT, o, o, HEIGHT + WALL, SKY_MAT),    # ceiling
        box(-o, -o, 0, -half, o, HEIGHT, SKY_MAT),            # -x wall
        box(half, -o, 0, o, o, HEIGHT, SKY_MAT),              # +x wall
        box(-half, -o, 0, half, -half, HEIGHT, SKY_MAT),      # -y wall
        box(-half, half, 0, half, o, HEIGHT, SKY_MAT),        # +y wall
    ]
    edge = half - 768
    spots = [(edge, edge), (-edge, edge), (edge, -edge), (-edge, -edge)]
    spots += [p for p in [(16384, 0), (-16384, 0), (0, 16384), (0, -16384), (24000, 24000)]
              if max(abs(p[0]), abs(p[1])) + 64 < half]
    for x, y in spots:
        solids.append(box(x - 64, y - 64, 0, x + 64, y + 64, 256, PILLAR_MAT))
    world = ('world\n{\n\t"id" "1"\n\t"mapversion" "1"\n\t"classname" "worldspawn"\n'
             '\t"skyname" "sky_day01_01"\n\t"maxpropscreenwidth" "-1"\n' + "".join(solids) + "}\n")
    ents = entity("info_player_start", (-edge + 256, -edge + 256, 8), angles="0 45 0")
    ents += entity("light_environment", (0, 0, 512), angles="-60 30 0", pitch="-60",
                   _light="255 255 255 200", _ambient="128 128 128 60")
    head = ('versioninfo\n{\n\t"editorversion" "400"\n\t"editorbuild" "0"\n\t"mapversion" "1"\n'
            '\t"formatversion" "100"\n\t"prefab" "0"\n}\n')
    with open(path, "w", newline="\n") as f:
        f.write(head + world + ents)
    print(f"WROTE {path}: interior +-{half} X/Y, 0..{HEIGHT} Z, outer +-{o}")


if __name__ == "__main__":
    if len(sys.argv) not in (2, 3):
        sys.exit(__doc__)
    write(sys.argv[1], int(sys.argv[2]) if len(sys.argv) == 3 else 31744)
