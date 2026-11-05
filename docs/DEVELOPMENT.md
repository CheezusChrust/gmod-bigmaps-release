# Development

The module targets stock binaries from game app 4000 public build `25375506`
and dedicated-server app 4020 public build `25375525`. It validates identities
and expected memory before patching; do not assume a later build is compatible.
The patch table in [sites.h](../module/common/sites.h) is specific to those
files. Detailed site provenance and historical analysis remain in the separate
research project.

## Build on Windows

Install Visual Studio 2022 C++ tools for x86 and x64 and Windows SDK
`10.0.19041.0`. From the repository root:

```powershell
.\tools\build-module.cmd
```

Outputs are `work\module\bigmaps\x86\gmsv_bigmaps_win32.dll` and
`work\module\bigmaps\x64\gmsv_bigmaps_win64.dll`; each directory also has an
inspection report. The main-menu Lua state loads the `gmsv_` module.

## Build for Linux x86

Use a Linux or WSL environment with `g++`, 32-bit multilib, `file`, `nm`,
`readelf`, `objdump` and `sha256sum`. Ubuntu 20.04 with `g++-multilib` was
used for the tested build. Build against a glibc no newer than the target server.
From the repository root in Linux:

```sh
sh tools/build-module-linux.sh
```

The output is `work/module/bigmaps/linux/gmsv_bigmaps_linux.dll` (GMod uses
the `.dll` extension for Linux binary modules). An inspection report lists
exports, dependencies, glibc symbols and the hash.

## Implementation and updates

The module verifies file hashes, patch sites, SendProps and engine anchors before
writing. It chooses the state from the pending BSP's world bounds, changes the
network SendProps and CRC, and restores stock behavior for normal maps. Windows
game hooks also synchronize the live `r_mapextents` value. Server autorun happens
too late for an oversized first map.

An update requires identifying the new binary builds, revalidating all sites and
anchors for each affected platform, rebuilding and testing. Do not just replace
hashes or disable verification. The [development tools](../tools/README.md)
include read-only porting/inspection utilities, reference signatures, emulators
and a module test harness. They do not automatically port the module's runtime
anchors or regenerate its header. The old header generator depends on the
retired DLL file-patching workflow and remains in the research project alongside
detailed patch evidence and historical test records.
See [testing](TESTING.md) for what has and has not been validated.

The [compiler patcher](../compiler/README.md) is independent of the runtime.
Do not commit game binaries, test installations, build outputs or secrets.
