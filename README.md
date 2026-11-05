# Garry's Mod Bigger Maps

An experimental binary module for playing maps beyond Garry's Mod's usual
±16,384-unit world limit. It targets approximately ±32,768 units per axis and
automatically switches between expanded and normal behavior for each map. It
patches the running process, not the game files on disk.

## Target platforms

The project targets the **latest stable/public** Garry's Mod builds for:

- Windows clients: 32-bit (x86) and 64-bit (x86-64).
- Windows dedicated servers: 32-bit (x86) and 64-bit (x86-64).
- Linux dedicated servers: 32-bit (x86).

**Compatibility is build-specific, not automatic.** The current module was tested
with Steam app 4000 public build `25375506` and app 4020 public build `25375525`.
It verifies stock binary hashes and refuses unknown builds. A later update may
require a new port even when it is on the stable branch. Linux clients and Linux
64-bit servers are not supported.

## Getting started

This is an initial testing project, not a finished release. Module binaries are
not stored in this repository; use a compatible release when one is published
or [build from source](docs/DEVELOPMENT.md).

1. [Install the module](docs/INSTALL.md) on your game or dedicated server.
2. On oversized multiplayer maps, the server and every client need the module
   and compatible stock files. Normal maps return to stock behavior.
3. Map authors can find the separate [vbsp++ patcher](compiler/README.md).

The safe boundary is not established, and z-fighting is worse near map edges.
Dedicated servers must start on a normal-sized map before changing to an
oversized one. See [testing and limitations](docs/TESTING.md).

## Development

The runtime source is in [module/](module/); build scripts are in [tools/](tools/).
See [development notes](docs/DEVELOPMENT.md) and [agent guidance](AGENTS.md).
The earlier research project is maintained separately and contains detailed
patch evidence and historical experiments.
