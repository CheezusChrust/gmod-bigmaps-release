# Windows x86-64 branch analysis

Status: an experimental Windows x64 game module has been built and loaded at
the main menu. Initial verification, both hook installations and normal-map
classification, direct oversized-map loading and patch activation passed.
The owner reports gameplay functioning properly in the oversized map.
Oversized-to-normal stock restoration and normal-to-oversized re-enabling
passed in the same session.
Extended transition cycling and multiplayer remain unverified.
The public-branch module remains separate and unchanged.

## Build identity

The app 4000 Steam manifest reported branch `x86-64`, build `26439081`.
Candidate Windows 64-bit DLLs were copied from the installed `bin/win64` into
the ignored `work/analysis/26439081-x86-64` directory and verified against their
source files:

| File | SHA-256 |
|---|---|
| `engine.dll` | `4525B25701DD14EB7C2D4EA49A559797A41BBC07E8D73F631C711F1B809DBDBA` |
| `client.dll` | `D6A468AB5FF462D17F7F6A4F6AE10910F1513B17CC1D0C5D1AE4712D88180C35` |
| `server.dll` | `FFFF73124BF46ADD691459A030C3426B940787229D1F3EF9640CE7DFE9BB1F96` |
| `filesystem_stdio.dll` | `814373EA10B7E4E4F48C1EAF820F35E7FED2A5EF756E77F0FB2D68D1323CE1AD` |

The older build loaded the game DLLs from `garrysmod/bin/win64`; that directory
is absent in this installation. Read-only inspection of the running 64-bit game
confirmed that all four files above were loaded from `bin/win64`. The analysis
copies place client and server in the old layout solely for the scanner.

## Site discovery

The initial public-build x64 signature scan reported 23 VERIFIED, 5 REVIEW and
29 NOT FOUND of 57 sites. The ignored `port-report.json` records that output;
its `build` field identifies the reference signature set, not the target build.
The five review sites were inspected in context; missing sites were re-derived
for this build. The generated target has 69 static patch spans plus one dynamic
sound hook. All 69 stock spans matched the running game's memory.

- The partition hash initialization is around `0x180216590`; the constructor
  still produces four levels, with a new `0x2090` level stride. Changed vector
  readers clamp spatial bounds; all axes were checked.
- Standard coordinate writer `0x1802B1FD0` and reader `0x1802B3180`; multiplayer
  writer `0x1802B2120` and reader `0x1802B3510`; SendProp skip `0x18019E7C0`
  and comparison `0x18019E480`. Their bit-buffer layouts differ from the old
  build. The codec probe records the exact instruction bytes.
- Sound writer coordinates are in `0x18014F455..0x18014F606`; the corresponding
  inlined reader block is `0x180211929..0x180211D64`. Its `r13d` width is also
  used by an earlier unrelated field at `0x180210BA1`. The branch-only sound
  trampoline increments it at the coordinate block instead, replays displaced
  instructions, and is verified in isolated emulation. The complete sound
  function has not been tested in game.
- CRC function `0x1801AB950` is called and stored at `0x1801AC26D`, to global
  `0x1808AE5F4`; its anchors matched live memory and the stored stock CRC was
  `0x546B7A01`. The successful menu initialization also implies the module
  reproduced the stored CRC before installing hooks.
- Host hook starts at `0x18015AEF0`; client sign-on hook starts at `0x1800AA370`
  with state and map-name offsets `0x15C` and `0x1D8`. The branch-specific client
  trampoline replays the RIP-relative load through its absolute target. Hook
  addresses and original prologues matched live memory, and the owner reported
  successful installation of both hooks at the main menu. The host hook ran for
  `gm_construct` and classified it as normal; the client join hook remains
  untested.
- RTTI and the `VBaseFileSystem011` factory indicate a secondary
  filesystem vtable at RVA `0xAFB88`, with first five slot RVAs `0x205D0`,
  `0x24980`, `0x1ECA0`, `0x14630`, `0x22BB0`; these matched live memory. The
  successful menu initialization implies the factory result passed module checks.
- The client ConVar stores numeric values XOR-encoded with the low 32 bits of
  its object address. The branch copy encodes live `r_mapextents` writes; its
  stock object, pointer layout and decoded values matched live memory.

## Offline checks

Both probes check the engine SHA-256 and modify only emulated memory. Run from
the repository root with the documented tooling dependencies installed:

```powershell
python module-x86-64\tests\probe_coord.py
python module-x86-64\tests\probe_sound.py
python module-x86-64\tests\probe_quantized.py
```

- Coordinate probe: 234 individual cases and three packed sequences passed for
  each of four modes at both stock 14-bit and candidate 15-bit widths. It checks
  independent bitstreams, decoded values, skip and comparison positions, and
  equality comparisons. This is not exhaustive proof of network compatibility.
- Sound probe: 288 cases each passed for stock and candidate widened reader and
  writer blocks, including changed-axis combinations, cross-word offsets and
  the widened reader trampoline. It does not execute the full sound function.
- Quantized probe: 771 cases each passed for stock and widened beam and effect
  properties, using the branch's server multiplier and engine encode/decode/skip.
- Native checks passed for hook bytes, relocated addresses, ConVar encoding and
  stock/patched/tampered/restored classification of all 69 static sites and the
  dynamic sound hook. Live stock SendTable fields and CRC anchors were checked.

The owner reported a successful main-menu load with all three game DLLs
recognized as stock, both hooks installed and the module currently stock. On
`gm_construct`, the host hook reported normal bounds and left the module stock.
The owner loaded `bm_features` directly from the menu: the module
reported 70 sites and seven SendProps written, zero failures, all-patched re-check,
CRC `0x546B7A01 -> 0x8D332B66` and `r_mapextents` `16384 -> 32768`. The owner
subsequently reported that everything gameplay-wise appeared to function
properly. Record this as an owner-reported single-player gameplay pass, not an
exhaustive per-feature or all-axis boundary certification. The compiler is
independent and outside this port's scope.

On `bm_features -> gm_construct`, the module restored 70 sites and seven
SendProps with zero failures and an all-stock re-check, changed CRC
`0x8D332B66 -> 0x546B7A01`, and reset `r_mapextents` to 16384. On
`gm_construct -> bm_features`, re-enabling without restarting also passed:
70 sites and seven SendProps, zero failures, all-patched re-check, CRC
`0x546B7A01 -> 0x8D332B66`, and `r_mapextents` restored to 32768.

## Remaining work

Optional follow-up: extended normal/oversized map cycles. Detailed boundary
and per-feature coverage has not been recorded. Live
refusal behavior, multiplayer and cross-build compatibility are unverified. This
branch module deliberately refuses dedicated-server use.