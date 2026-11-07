# Development

The module targets stock binaries from game app 4000 public build `25375506`
and dedicated-server app 4020 public build `25375525`. It validates identities
and expected memory before patching; do not assume a later build is compatible.
The per-build patch tables in [targets/](../module/targets/) are specific to
those files. Detailed site provenance and historical analysis remain in the
separate research project.

## Source layout

The module is built as one translation unit, with shared code separated by
responsibility and build-specific data kept out of the platform logic:

- [targets/targets.h](../module/targets/targets.h) defines the records and
  selects exactly one target: [win32.h](../module/targets/win32.h),
  [win64.h](../module/targets/win64.h), or
  [linux_x86.h](../module/targets/linux_x86.h). Each target holds the supported
  file hashes, patch sites and anchors, plus platform-specific RVAs and layout
  offsets. Do not mix target files or edit an address without revalidating it
  against the corresponding stock binary.
- [common/platform.h](../module/common/platform.h) selects the OS layer:
  [platform_win.h](../module/common/platform_win.h) or
  [platform_linux.h](../module/common/platform_linux.h). These provide the
  memory, file identity, and module lookup primitives needed by the shared
  code. Linux x86 remains dedicated-server-only; Windows x86 and x64 also
  support the game hooks.
- The remaining [common/](../module/common/) headers separate patch-site
  validation, SendTable access, engine state/CRC, map and BSP checks, logging,
  and Linux SHA-256.
- [bigmaps.cpp](../module/bigmaps/bigmaps.cpp) contains the Lua module entry
  points. [patcher.h](../module/bigmaps/patcher.h) verifies and switches the
  patch state; [game_hooks_win.h](../module/bigmaps/game_hooks_win.h) contains
  the Windows game-only hooks.

Each build compiles `bigmaps.cpp`; the headers are intentionally split for
review and maintenance, not linked as separate binaries.

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
anchors or regenerate target data. The old header generator depends on the
retired DLL file-patching workflow and remains in the research project alongside
detailed patch evidence and historical test records. The included offline and
mocked checks are documented under [regression tests](../tools/README.md#regression-tests);
live server tests require the disposable copies and owner-operated map changes
described under [module testing](../tools/README.md#module-testing-windows).

The [compiler patcher](../compiler/README.md) is independent of the runtime.
Do not commit game binaries, test installations, build outputs or secrets.
