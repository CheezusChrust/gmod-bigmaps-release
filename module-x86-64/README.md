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
offline emulator probes pass. This build has **not** been loaded in the game;
the earlier single-player and map-transition results apply only to build
26439081. See [the port analysis](../docs/X86_64_PORT_ANALYSIS.md) for hashes,
evidence and the remaining live-test requirements.

Before installing, preserve the current module and close the game. Do not mix
this branch module with the public-branch module. After loading, verify the
`[bigmaps]` messages, normal-map classification and oversized-map activation;
test movement, traces, props, effects and sound beyond the old boundary. Map
transitions, multiplayer, cross-build compatibility and dedicated servers are
not verified for build 26623711.