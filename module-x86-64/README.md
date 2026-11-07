# Experimental x86-64 branch module

This separate Windows 64-bit game module targets Steam app 4000 `x86-64` build
`26439081`. It is not the public-branch module and refuses dedicated-server use.
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

Offline and read-only checks have passed, but this build has not yet been loaded
or tested in game. Start with a normal map, then the existing oversized test map;
check the `[bigmaps]` console messages and test movement, traces, props, effects
and sound beyond the old boundary. Multiplayer, cross-build compatibility and
dedicated servers are unverified. See [the port analysis](../docs/X86_64_PORT_ANALYSIS.md)
for evidence and reproducible emulator checks.