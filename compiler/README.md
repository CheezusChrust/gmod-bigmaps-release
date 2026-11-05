# vbsp++ patcher

This separate tool patches one specific Windows x64 vbsp++ executable so it can
compile oversized maps. It does not patch stock GMod vbsp. The runtime module
loads precompiled maps without this tool.

## Patch a copy

The supported input is the `VBSP++ Oct 3 2026` build, SHA-256:

```text
80289A94437E6A8CB96F74D14B0EF86ED78A28465A4862241D02F2816C120D92
```

With Python 3 installed, copy the original executable out of your game/tool
installation. From the repository root, for example:

```powershell
python .\compiler\patch_vbsp.py .\work\vbsp-original\vbsp++.exe .\work\vbsp-patched\vbsp++.exe
```

The input hash and every original patch byte must match. The output must not
already exist; the tool never overwrites it or the input. Configure your own
compiler workflow to use the patched copy. Tests used unmodified vvis++ and
vrad++. No compiler binary is included in this repository.

Generated test maps compiled and played successfully; an author-created
Hammer++ map and editing beyond the old boundary remain untested. Exact safe
map boundaries are not established. See [testing](../docs/TESTING.md).

## Maintaining the patch

The changes cover node/leaf signed-short saturation, world bounds and boxes,
the water sentinel, portal limit, block grid and vertex hash. A new vbsp++
build needs fresh binary analysis and verification: addresses and bytes can
change. Do not bypass the hash or byte checks. Compiler signatures are included
in [sites_vbsppp-80289A94.json](sites_vbsppp-80289A94.json); use the shared
[port-site tool](../tools/README.md) against a copy with the
`bin\win64\vbsp++.exe` layout:

```powershell
python tools\port_sites.py find work\analysis\<compiler-build> compiler\sites_vbsppp-80289A94.json
python tools\port_sites.py gen work\analysis\vbsppp-80289A94 work\sites-vbsp.json --target vbsppp
```

The `gen` command only refreshes the exact reference build's signatures.
The separate research project retains detailed patch evidence. Before redistributing a compiler binary, independently
confirm that you have the rights to do so.

## Test maps

These Python tools use only the standard library:

```powershell
python compiler\make_test_vmf.py work\room.vmf
python compiler\make_feature_vmf.py work\bm_features.vmf
python compiler\inspect_bsp.py work\bm_features.bsp
```

Create `work` first if it does not exist. Compile the generated VMF using your
own compiler setup; these scripts do not run a compiler or install a map.
[make_test_vmf.py](make_test_vmf.py) accepts an optional room half-width
(default 31744), while [make_feature_vmf.py](make_feature_vmf.py) generates
the feature stations described in the [in-game checklist](bm_features-checklist.md).
[inspect_bsp.py](inspect_bsp.py) reports bounds and inverted node/leaf boxes;
give it a second BSP to compare lumps.
