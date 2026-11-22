# Experimental x86-64 branch module

This separate Windows 64-bit game module targets Steam app 4000 `x86-64` build
`26623711`. It is not the public-branch module and refuses dedicated-server use.
It verifies exact DLL hashes, patch bytes, SendProps, CRC anchors and the game
filesystem before writing. A different game update requires a new port.

Build from the repository root with Visual Studio 2022 C++ tools and Windows SDK
`10.0.19041.0`:

```powershell
.\tools\build-module-x86-64.cmd
.\tools\test-module-x86-64.cmd
```

The experimental output is `work\module-x86-64\bigmaps\x64\gmsv_bigmaps_win64.dll`.
It has the same filename as the public module; preserve the public DLL before
replacing it and close the game before switching versions. The existing main-menu
`require("bigmaps")` loader is used. Do not install both versions at once.

The 26623711 target signatures, anchors, hooks and filesystem-provider hash
match staged copies of the installed binaries. The native tests and three
offline emulator probes pass. The owner reports that `bm_features` works
in-game and that a small -> large -> small map cycle applies and restores the
module's patch state correctly. This does not certify every feature, boundary,
multiplayer or dedicated-server behavior. The earlier detailed single-player
results apply only to build 26439081. See [the port analysis](../docs/X86_64_PORT_ANALYSIS.md)
for hashes, evidence and remaining coverage gaps.

The module also hooks the Lua enum function in `server.dll` and `client.dll` (RVAs 0x104750 and
0x8B0F0 for this build) to set `MIN_COORD_FLOAT` and `MAX_COORD_FLOAT` in every Lua state before
any addon runs (-16384/16384 on normal maps, -32768/32768 on oversized ones). The public-branch
module's `-usegh` dedicated-server mode is not part of this game-only module.

Before installing another version, preserve the current module and close the
game. Do not mix this branch module with the public-branch module. The reported
test covered `bm_features` and a small-large-small transition cycle; movement,
traces, props, effects, sound, exact boundaries, multiplayer, cross-build
compatibility and dedicated servers are not all independently certified for
build 26623711.

The `LV-xx` sites (16) widen the `mat_leafvis` leaf wireframe's world box from +-16384 to +-32768 (`AddLeafPortals`, engine.dll RVA 0xBFB20). They are verified against the stock bytes and decode correctly when applied to a copy; tested in-game.
