# Development tools

Run these from the repository root with Python 3.10 or newer. Install the optional
Python tooling dependencies with `python -m pip install -r tools\requirements.txt`.
The module build itself does not need these packages.

## Porting and inspection (read-only)

- [port_sites.py](port_sites.py): locate patch sites after a game/compiler update,
  inspect cross-references, or refresh reference signatures.
- [pe_find.py](pe_find.py): inspect PE identities, sections, bytes and addresses.
- [elf_image.py](elf_image.py): shared Linux x86 ELF support for inspection/emulation.

Copy binaries into `work\analysis\<build>` with their installation-relative
layout; never analyze by modifying the game installation. For example:

```powershell
python tools\port_sites.py find work\analysis\<build> tools\sites_25375506.json --json work\report-x64.json
python tools\port_sites.py find work\analysis\<build>-x86 tools\sites_25375506-x86.json --json work\report-x86.json
python tools\port_sites.py xrefs work\analysis\<build>\bin\win64\engine.dll 0xADDRESS
python tools\pe_find.py work\analysis\<build>\bin\win64\engine.dll info
```

The x64 layout is `bin\win64\engine.dll` plus
`garrysmod\bin\win64\client.dll` and `server.dll`; x86 omits `win64`.
Linux x86 `xrefs FILE VA [--data]` and `sym FILE TEXT|VA` are supported, but
signature generation/finding is Windows PE-only.

`find` reports VERIFIED, REVIEW, MISMATCH or NOT FOUND. Exit codes are **0** for
all verified, **2** when review is needed, **1** for unresolved/mismatched sites.
REVIEW includes ambiguous occurrence-order matches, loose matches, recovered
tables/immediates and changed shared-constant readers. Inspect each before using
it. Do not equate a located signature with a validated port: confirm function
semantics, all readers of shared literals, instruction boundaries and relocation
safety. Keep the four-level spatial partition and matching network readers,
writers and skip paths; do not globally replace every occurrence of 16384.

The tool only emits proposals and reports; it does not patch DLLs or update the
module table. The module's CRC, host-state, filesystem and hook anchors need
separate validation too. `gen` refreshes signatures from the bundled metadata
and exact hash-verified reference binaries, without a DLL-patcher dependency:

```powershell
python tools\port_sites.py gen work\analysis\25375506 work\sites-x64.json
python tools\port_sites.py gen work\analysis\25375506-x86 work\sites-x86.json --target gmod-x86
```

This refresh is not an automatic new-build port. Keep generated output separate
from the bundled reference JSON until it has been reviewed.

## Offline emulation

[emulate_codec.py](emulate_codec.py) exercises the engine's coordinate codec;
[emulate_quantized.py](emulate_quantized.py) exercises the beam/effect SendProps
and can cross-check Windows architectures. Both use Unicorn and read supplied
copies without starting the game.

Their addresses are fixed to Windows build `25375506`; the coordinate emulator
also supports Linux x86 server build `25375525`. They do not automatically
consume port-site reports. Revalidate their addresses before using another build.
The coordinate test expects expanded ranges: a stock engine intentionally fails
those checks and is a negative control.

```powershell
python tools\emulate_codec.py work\analysis\25375506\bin\win64\engine.dll
python tools\emulate_quantized.py work\analysis\25375506\bin\win64\engine.dll work\analysis\25375506\garrysmod\bin\win64\server.dll
```

## Module testing (Windows)

[test_module.py](test_module.py) uses a **disposable stock server copy** in
`work\gmod-ds` (x64) or `work\gmod-ds-x86` (x86). It installs/removes its own
module and Lua loader files there; do not use a production installation or place
unrelated customizations at those paths. Build the module first, then:

```powershell
python tools\test_module.py x64
python tools\test_module.py x64 --log
```

The server starts on `gm_construct`, port 27025. Provide any test maps yourself.
The harness never sends console commands or stops the server. Perform map
changes/joins manually and close it yourself with `quit`; `--log` reads the
latest console segment and cleans test files once the port is no longer in use.
The obsolete probe module is not required.

## Regression tests

```powershell
python -m unittest discover -s tools\tests -v
```

Binary-backed tests skip if the reference copies are absent. Set
`BIGMAPS_ANALYSIS_ROOT` to a directory containing `25375506`, `25375506-x86`,
`26444477`, `26444477-x86` and `vbsppp-80289A94` to run the full suite.
Bundled-metadata checks and mocked harness tests run without those binaries.
The harness tests do not start a server.

Map generation, BSP inspection and compiler signatures live separately under
[compiler/](../compiler/README.md). Historical DLL file-patch builders, their
dependent header generator, probe binaries, initial toolchain exercises and
machine-specific launchers are intentionally not included.
