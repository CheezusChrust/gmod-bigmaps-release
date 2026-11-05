"""Write a feature test map (VMF) for the +-32768 build: one or more of each feature worth checking
far outside the stock +-16384 limit. Reuses the brush helpers from make_test_vmf.py.

Interior: X/Y +-31744, Z 0..30720 (sky walls/ceiling), plus a well down to Z -30720.
Stations (centres; in-game checklist: compiler/bm_features-checklist.md):
  S  (0, 0)              spawn, control props near the old range
  A  (+28672, +28672)    displacement hill, overlay, decal, cubemap, static/physics props,
                         func_detail stairs, func_rotating, breakable glass, point light
  B  (-28672, +28672)    water pool with env_bubbles, floating barrels, cubemap
  C  (+28672, -28672)    brush ladder (toolsinvisibleladder) and func_useableladder on a tower
  D  (-28672, -28672)    env_beam, env_laser, env_spark, env_sprite, env_smokestack,
                         point_spotlight, light_spot, looping ambient_generic, weapons + ammo
  T  x 12000..20000      func_tracktrain loop crossing x = +16384; env_beam crossing x = -16384
  H  (0, +24576, 29696)  high platform near +Z
  W  (0, -24576)         well down to Z -30720 with water at the bottom
  E  (+-31232, 0) etc.   edge pillars; ambient sound near the +X wall

Usage: python compiler/make_feature_vmf.py OUT.vmf
"""
import sys

from make_test_vmf import entity, nid, outward

HALF = 31744
WALL = 64
TOP = 30720
WELL_BOTTOM = -30720
WELL = (-512, -25088, 512, -24064)  # x1, y1, x2, y2 of the well opening

DEV = "DEV/DEV_MEASUREGENERIC01B"
PILLAR = "DEV/DEV_MEASUREWALL01A"
SKY = "TOOLS/TOOLSSKYBOX"
NODRAW = "TOOLS/TOOLSNODRAW"
CONCRETE = "CONCRETE/CONCRETEWALL008A"
METAL = "METAL/METALWALL001A"
WATER = "NATURE/WATER_CANALS_WATER2"
LADDER = "TOOLS/TOOLSINVISIBLELADDER"
TRIGGER = "TOOLS/TOOLSTRIGGER"
ORIGIN = "TOOLS/TOOLSORIGIN"
GLASS = "GLASS/GLASSWINDOW007A"
BLEND = "NATURE/BLENDROCKDIRT008A"


def axes(p):
    if p[0][2] == p[1][2] == p[2][2]:
        return "[1 0 0 0] 0.25", "[0 -1 0 0] 0.25"
    if p[0][0] == p[1][0] == p[2][0]:
        return "[0 1 0 0] 0.25", "[0 0 -1 0] 0.25"
    return "[1 0 0 0] 0.25", "[0 0 -1 0] 0.25"


def dispinfo(start, power=3, height=384):
    n = 2 ** power + 1
    rows = lambda f: "".join(f'\t\t\t\t"row{i}" "{f(i)}"\n' for i in range(n))
    def dist(i, j):
        r2 = ((i - (n - 1) / 2) ** 2 + (j - (n - 1) / 2) ** 2) / ((n - 1) / 2) ** 2
        return max(0.0, height * (1 - r2))
    blk = lambda name, f: f"\t\t\t{name}\n\t\t\t{{\n" + rows(f) + "\t\t\t}\n"
    out = (f'\t\t\tdispinfo\n\t\t\t{{\n\t\t\t"power" "{power}"\n'
           f'\t\t\t"startposition" "[{start[0]} {start[1]} {start[2]}]"\n'
           '\t\t\t"flags" "0"\n\t\t\t"elevation" "0"\n\t\t\t"subdiv" "0"\n')
    out += blk("normals", lambda i: " ".join(["0 0 1"] * n))
    out += blk("distances", lambda i: " ".join("%g" % dist(i, j) for j in range(n)))
    out += blk("offsets", lambda i: " ".join(["0 0 0"] * n))
    out += blk("offset_normals", lambda i: " ".join(["0 0 1"] * n))
    out += blk("alphas", lambda i: " ".join("%g" % min(255, 255 * j / (n - 1)) for j in range(n)))
    out += "\t\t\ttriangle_tags\n\t\t\t{\n" + "".join(
        f'\t\t\t\t"row{i}" "{" ".join(["9"] * (2 * (n - 1)))}"\n' for i in range(n - 1)) + "\t\t\t}\n"
    out += '\t\t\tallowed_verts\n\t\t\t{\n\t\t\t\t"10" "' + " ".join(["-1"] * 10) + '"\n\t\t\t}\n'
    return out + "\t\t\t}\n"


class Box:
    """Axis-aligned brush; mat is the default, top overrides the +Z face. Records the top side id."""

    def __init__(self, x1, y1, z1, x2, y2, z2, mat, top=None, scale=16, disp=None):
        self.b = (x1, y1, z1, x2, y2, z2)
        self.mat, self.topmat, self.scale, self.disp = mat, top or mat, scale, disp
        self.top_id = None

    def text(self):
        x1, y1, z1, x2, y2, z2 = self.b
        faces = [
            ("top", (x1, y2, z2), (x2, y2, z2), (x2, y1, z2)),
            ("bottom", (x1, y1, z1), (x2, y1, z1), (x2, y2, z1)),
            ("-x", (x1, y2, z2), (x1, y1, z2), (x1, y1, z1)),
            ("+x", (x2, y2, z1), (x2, y1, z1), (x2, y1, z2)),
            ("+y", (x2, y2, z2), (x1, y2, z2), (x1, y2, z1)),
            ("-y", (x2, y1, z1), (x1, y1, z1), (x1, y1, z2)),
        ]
        c = ((x1 + x2) / 2, (y1 + y2) / 2, (z1 + z2) / 2)
        out = f'\tsolid\n\t{{\n\t\t"id" "{nid()}"\n'
        for name, *pts in faces:
            p = outward(tuple(pts), c)
            sid = nid()
            if name == "top":
                self.top_id = sid
            mat = self.topmat if name == "top" else self.mat
            ux, vx = axes(p)
            out += (f'\t\tside\n\t\t{{\n\t\t\t"id" "{sid}"\n'
                    f'\t\t\t"plane" "{" ".join("(%g %g %g)" % q for q in p)}"\n'
                    f'\t\t\t"material" "{mat}"\n\t\t\t"uaxis" "{ux}"\n\t\t\t"vaxis" "{vx}"\n'
                    f'\t\t\t"rotation" "0"\n\t\t\t"lightmapscale" "{self.scale}"\n'
                    '\t\t\t"smoothing_groups" "0"\n')
            if name == "top" and self.disp:
                out += dispinfo((x1, y1, z2), height=self.disp)
            out += "\t\t}\n"
        return out + "\t}\n"


def ent(cls, origin=None, solids=(), connections=None, **kv):
    if not solids:
        e = entity(cls, origin, **kv)
        if connections:
            e = e[:-2] + conn(connections) + "}\n"
        return e
    body = "".join(f'\t"{k}" "{v}"\n' for k, v in kv.items())
    if origin is not None:
        body += f'\t"origin" "{origin[0]} {origin[1]} {origin[2]}"\n'
    return (f'entity\n{{\n\t"id" "{nid()}"\n\t"classname" "{cls}"\n{body}'
            + "".join(s.text() for s in solids) + (conn(connections) if connections else "") + "}\n")


def conn(items):
    return "\tconnections\n\t{\n" + "".join(f'\t\t"{k}" "{v}"\n' for k, v in items) + "\t}\n"


def light(o, rgb="255 240 220", bright=400):
    return ent("light", o, _light=f"{rgb} {bright}", _constant_attn="0", _linear_attn="0",
               _quadratic_attn="1")


def prop_static(o, model, ang="0 0 0"):
    return ent("prop_static", o, model=model, angles=ang, solid="6", skin="0", fademindist="-1",
               fadescale="1")


def prop_physics(o, model, ang="0 0 0"):
    return ent("prop_physics", o, model=model, angles=ang, spawnflags="256", fademindist="-1",
               fadescale="1")


def build():
    o = HALF + WALL
    wx1, wy1, wx2, wy2 = WELL
    world, ents = [], []
    # Floor (four slabs around the well opening); the north slab carries the overlay.
    north = Box(-o, wy2, -WALL, o, o, 0, DEV, scale=64)
    world += [north, Box(-o, -o, -WALL, o, wy1, 0, DEV, scale=64),
              Box(-o, wy1, -WALL, wx1, wy2, 0, DEV, scale=64), Box(wx2, wy1, -WALL, o, wy2, 0, DEV, scale=64)]
    # Sky shell.
    world += [Box(-o, -o, TOP, o, o, TOP + WALL, SKY), Box(-o, -o, 0, -HALF, o, TOP, SKY),
              Box(HALF, -o, 0, o, o, TOP, SKY),
              Box(-HALF, -o, 0, HALF, -HALF, TOP, SKY),
              Box(-HALF, HALF, 0, HALF, o, TOP, SKY)]
    # Well: walls, bottom, water, light.
    zb = WELL_BOTTOM
    world += [Box(wx1 - WALL, wy1 - WALL, zb - WALL, wx2 + WALL, wy2 + WALL, zb, CONCRETE, scale=64),
              Box(wx1 - WALL, wy1 - WALL, zb, wx1, wy2 + WALL, -WALL, CONCRETE, scale=64),
              Box(wx2, wy1 - WALL, zb, wx2 + WALL, wy2 + WALL, -WALL, CONCRETE, scale=64),
              Box(wx1, wy1 - WALL, zb, wx2, wy1, -WALL, CONCRETE, scale=64),
              Box(wx1, wy2, zb, wx2, wy2 + WALL, -WALL, CONCRETE, scale=64),
              Box(wx1, wy1, zb, wx2, wy2, zb + 512, NODRAW, top=WATER)]
    ents += [light((0, -24576, zb + 768), bright=800), light((0, -24576, -15360), bright=800),
             prop_physics((0, -24576, zb + 640), "models/props_borealis/bluebarrel001.mdl"),
             ent("env_cubemap", (0, -24576, zb + 600), cubemapsize="0")]

    # Edge pillars and corner pillars.
    e = HALF - 512
    for x, y in [(e, 0), (-e, 0), (0, e), (0, -e), (e, e), (-e, e), (e, -e), (-e, -e)]:
        world.append(Box(x - 64, y - 64, 0, x + 64, y + 64, 256, PILLAR))
    ents.append(ent("ambient_generic", (e + 256, 0, 64), message="ambient/atmosphere/city_beacon_loop1.wav",
                    health="10", radius="3000", pitch="100", pitchstart="100", spawnflags="0"))

    # S: spawn and control props near the old range.
    ents += [ent("info_player_start", (0, 0, 8), angles="0 45 0"),
             ent("light_environment", (0, 0, 512), angles="-60 30 0", pitch="-60",
                 _light="255 255 255 200", _ambient="128 128 128 60"),
             prop_static((512, 512, 0), "models/props_c17/fence01a.mdl"),
             prop_physics((768, 512, 32), "models/props_junk/wood_crate001a.mdl"),
             light((640, 640, 256))]

    # A: displacement, overlay, decal, cubemap, props, detail stairs, rotating, glass.
    ax, ay = 28672, 28672
    world.append(Box(ax - 1024, ay - 1024, 0, ax, ay, 8, NODRAW, top=BLEND, scale=16, disp=384))
    world.append(Box(ax + 256, ay - 1024, 0, ax + 320, ay - 512, 256, CONCRETE))  # decal wall
    ents += [ent("info_overlay", (ax + 512, ay + 512, 0), material="overlays/sandtrack02b",
                 sides=str(north.top_id), RenderOrder="0", StartU="0", EndU="1", StartV="0", EndV="1",
                 BasisOrigin=f"{ax + 512} {ay + 512} 0", BasisU="1 0 0", BasisV="0 1 0",
                 BasisNormal="0 0 1", uv0="-256 -128 0", uv1="-256 128 0", uv2="256 128 0",
                 uv3="256 -128 0", fademindist="-1", fademaxdist="0")]
    ents += [ent("infodecal", (ax + 256, ay - 768, 128), texture="decals/lambdaspray_1a"),
             ent("env_cubemap", (ax + 128, ay + 128, 128), cubemapsize="0"),
             prop_static((ax + 384, ay + 128, 0), "models/props_c17/fence01a.mdl"),
             prop_static((ax + 512, ay - 256, 0), "models/props_c17/lamppost03a_off.mdl", "0 90 0"),
             prop_static((ax + 640, ay + 128, 0), "models/props_trainstation/trashcan_indoor001a.mdl"),
             prop_physics((ax + 384, ay + 384, 32), "models/props_junk/wood_crate001a.mdl"),
             prop_physics((ax + 448, ay + 384, 96), "models/props_junk/wood_crate001a.mdl"),
             prop_physics((ax + 640, ay + 384, 48), "models/props_c17/oildrum001.mdl"),
             light((ax + 256, ay + 256, 384), bright=600)]
    stairs = [Box(ax + 128, ay + 640 + 64 * i, 0, ax + 384, ay + 704 + 64 * i, 16 * (i + 1), CONCRETE)
              for i in range(6)]
    ents.append(ent("func_detail", solids=stairs))
    ents.append(ent("func_rotating", (ax + 768, ay + 768, 72),
                    solids=[Box(ax + 768 - 192, ay + 768 - 16, 64, ax + 768 + 192, ay + 768 + 16, 80, METAL),
                            Box(ax + 760, ay + 760, 64, ax + 776, ay + 776, 80, ORIGIN)],
                    targetname="spinner", maxspeed="60", fanfriction="20", spawnflags="1", rendercolor="255 255 255"))
    ents.append(ent("func_breakable", solids=[Box(ax + 896, ay - 512, 0, ax + 904, ay - 256, 192, GLASS)],
                    material="0", health="1", propdata="0", spawnflags="0", rendercolor="255 255 255"))

    # B: water pool with bubbles, floating barrels, cubemap.
    bx, by = -28672, 28672
    p1, p2 = 1024, 1024 + WALL
    world += [Box(bx - p2, by - p2, 0, bx + p2, by - p1, 512, CONCRETE), Box(bx - p2, by + p1, 0, bx + p2, by + p2, 512, CONCRETE),
              Box(bx - p2, by - p1, 0, bx - p1, by + p1, 512, CONCRETE), Box(bx + p1, by - p1, 0, bx + p2, by + p1, 512, CONCRETE),
              Box(bx - p1, by - p1, 0, bx + p1, by + p1, 448, NODRAW, top=WATER)]
    ents += [ent("env_bubbles", solids=[Box(bx - 256, by - 256, 0, bx + 256, by + 256, 448, TRIGGER)],
                 density="20", frequency="10", current="0", spawnflags="0"),
             ent("env_cubemap", (bx, by + 512, 512), cubemapsize="0"),
             prop_physics((bx + 256, by + 256, 480), "models/props_borealis/bluebarrel001.mdl"),
             prop_physics((bx - 256, by + 384, 480), "models/props_junk/wood_crate001a.mdl"),
             light((bx, by, 768), bright=600)]

    # C: ladders on a tower.
    cx, cy = 28672, -28672
    world.append(Box(cx - 256, cy - 256, 0, cx + 256, cy + 256, 1024, CONCRETE))
    world.append(Box(cx - 64, cy - 264, 0, cx + 64, cy - 256, 1024, LADDER))  # -Y face
    ents += [prop_static((cx - 64, cy - 268, 0), "models/props_c17/metalladder001.mdl", "0 270 0"),
             ent("func_useableladder", (cx + 272, cy, 0), point0=f"{cx + 288} {cy} 0",
                 point1=f"{cx + 288} {cy} 1024", StartDisabled="0", ladderSurfaceProperties="ladder"),
             prop_static((cx + 268, cy, 0), "models/props_c17/metalladder001.mdl"),
             prop_physics((cx, cy, 1064), "models/props_junk/wood_crate001a.mdl"),
             light((cx - 512, cy - 512, 512), bright=600)]

    # D: effects, sound, weapons.
    dx, dy = -28672, -28672
    ents += [ent("info_target", (dx, dy, 128), targetname="beam_a"),
             ent("info_target", (dx, dy + 1536, 512), targetname="beam_b"),
             ent("env_beam", (dx, dy, 128), targetname="beam1", LightningStart="beam_a", LightningEnd="beam_b",
                 texture="sprites/laserbeam.vmt", life="0", BoltWidth="4", NoiseAmplitude="3",
                 TextureScroll="35", StrikeTime="1", Radius="256", damage="0", renderamt="255",
                 rendercolor="100 200 255", spawnflags="1"),
             ent("info_target", (dx + 768, dy + 1536, 64), targetname="laser_target"),
             ent("env_laser", (dx + 768, dy, 64), LaserTarget="laser_target", texture="sprites/laserbeam.vmt",
                 width="2", damage="0", renderamt="255", rendercolor="255 40 40", spawnflags="1"),
             ent("env_spark", (dx - 512, dy, 96), MaxDelay="1", Magnitude="2", TrailLength="1", spawnflags="64"),
             ent("env_sprite", (dx - 512, dy + 512, 160), model="sprites/light_glow02_add.vmt", rendermode="9",
                 renderamt="255", rendercolor="255 200 100", scale="1", GlowProxySize="4", spawnflags="1"),
             ent("env_smokestack", (dx - 1024, dy, 0), InitialState="1", BaseSpread="20", SpreadSpeed="15",
                 Speed="40", StartSize="20", EndSize="60", Rate="20", JetLength="300",
                 SmokeMaterial="particle/SmokeStack.vmt", rendercolor="128 128 128", renderamt="255",
                 WindAngle="0 0 0", WindSpeed="0"),
             ent("point_spotlight", (dx + 512, dy - 512, 512), angles="60 90 0", spotlightlength="600",
                 spotlightwidth="50", rendercolor="255 255 220", spawnflags="1"),
             ent("light_spot", (dx, dy - 768, 512), angles="-90 0 0", pitch="-90", _light="255 200 120 800",
                 _cone="45", _inner_cone="30", _exponent="1", _quadratic_attn="1"),
             ent("ambient_generic", (dx + 1024, dy + 1024, 64),
                 message="ambient/levels/labs/machine_ring_resonance_loop1.wav", health="10", radius="2048",
                 pitch="100", pitchstart="100", spawnflags="0")]
    for i, cls in enumerate(["weapon_crossbow", "weapon_rpg", "weapon_smg1", "weapon_frag", "item_ammo_crossbow",
                             "item_rpg_round", "item_ammo_smg1_grenade"]):
        ents.append(ent(cls, (dx + 1536, dy - 512 + 128 * i, 16)))

    # T: train loop crossing x = +16384, beam crossing x = -16384.
    pts = [(12000, -1024), (20000, -1024), (20000, 1024), (12000, 1024)]
    for i, (x, y) in enumerate(pts):
        ents.append(ent("path_track", (x, y, 16), targetname=f"pt{i + 1}", target=f"pt{(i + 1) % 4 + 1}"))
    tx, ty = pts[0]
    ents.append(ent("func_tracktrain", (tx, ty, 16), targetname="train1", target="pt1", speed="400",
                    startspeed="400", orientationtype="1", wheels="50", height="0", spawnflags="2", dmg="0",
                    rendercolor="255 255 255",
                    solids=[Box(tx - 128, ty - 128, 8, tx + 128, ty + 128, 72, METAL),
                            Box(tx - 8, ty - 8, 8, tx + 8, ty + 8, 24, ORIGIN)]))
    ents.append(ent("logic_auto", (0, 64, 16), spawnflags="1",
                    connections=[("OnMapSpawn", "train1,StartForward,,1,-1")]))
    ents += [ent("info_target", (-20000, -8000, 256), targetname="xbeam_a"),
             ent("info_target", (-12000, -8000, 256), targetname="xbeam_b"),
             ent("env_beam", (-20000, -8000, 256), LightningStart="xbeam_a", LightningEnd="xbeam_b",
                 texture="sprites/laserbeam.vmt", life="0", BoltWidth="6", NoiseAmplitude="0",
                 TextureScroll="35", StrikeTime="1", Radius="256", damage="0", renderamt="255",
                 rendercolor="255 255 0", spawnflags="1")]

    # H: high platform near +Z.
    world.append(Box(-512, 24064, 29632, 512, 25088, 29696, CONCRETE, scale=32))
    ents += [prop_physics((0, 24576, 29728), "models/props_junk/wood_crate001a.mdl"),
             light((0, 24576, 30080), bright=600)]

    worldtext = ('world\n{\n\t"id" "1"\n\t"mapversion" "1"\n\t"classname" "worldspawn"\n'
                 '\t"skyname" "sky_day01_01"\n\t"maxpropscreenwidth" "-1"\n'
                 + "".join(b.text() for b in world) + "}\n")
    # Overlay "sides" needs the north slab's side id, which is assigned when the world text is built.
    head = ('versioninfo\n{\n\t"editorversion" "400"\n\t"editorbuild" "0"\n\t"mapversion" "1"\n'
            '\t"formatversion" "100"\n\t"prefab" "0"\n}\n')
    return head, worldtext, ents, north


if __name__ == "__main__":
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    head, worldtext, ents, north = build()
    text = head + worldtext + "".join(ents).replace('"sides" "None"', f'"sides" "{north.top_id}"')
    with open(sys.argv[1], "w", newline="\n") as f:
        f.write(text)
    print(f"WROTE {sys.argv[1]}: interior +-{HALF} X/Y, Z 0..{TOP}, well to {WELL_BOTTOM}; "
          f"overlay side {north.top_id}")
