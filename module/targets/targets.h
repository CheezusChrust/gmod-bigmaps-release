// Per-build data. Every address, offset and hash in a target file belongs to one exact set of stock files and is
// verified at run time before use. Porting to another game build means re-deriving the whole target file, not
// just its hashes (docs/DEVELOPMENT.md).
//   win64.h      Windows x64: game and dedicated server
//   win32.h      Windows x86: game and dedicated server
//   linux_x86.h  Linux x86 (srcds_linux): dedicated server only
#pragma once

#include <stddef.h>

namespace bm
{

// A byte range in one module at `rva` from its load base. `stock`/`patched` are the bytes at the module's
// preferred base; Expected() shifts relocated pointers inside them by the actual load delta. Anchors and hook
// prologues use the same array for both (code that is only checked, never patched).
struct ProbeSite {
    const char* module;
    const char* id;
    unsigned rva;
    unsigned len;  // at most 32
    const unsigned char* stock;
    const unsigned char* patched;
    unsigned relocMask; // bit i: a relocated value starts at byte i
};

// patchedSha identifies the retired file-patched copies, only so they can be named when refused.
struct ProbeFile {
    const char* module;
    const char* stockSha;
    const char* patchedSha;
    unsigned long long preferredBase;
};

// Linux only: ELF section headers are not mapped, so the image size and the ranges searched for strings
// (.rodata) and SendTables (writable data) are recorded from the files.
struct ProbeImage {
    const char* module;
    unsigned size, roRva, roSize, rwRva, rwSize;
};

// Where the engine's IBaseFileSystem comes from (map_check.h). pointerRva != 0: a global in the module holds the
// interface pointer; 0: ask the module's exported CreateInterface. vtableRva and slotRva[] (Read, Write, Open,
// Close, Seek) must match before the first call.
struct FsProvider { const char* module; const char* sha; unsigned pointerRva; unsigned vtableRva; unsigned slotRva[5]; };

// Windows game only: the r_mapextents ConVar object in client.dll and the field offsets inside it.
struct MapExtentsVar { unsigned objRva, nameRva, defaultRva, offName, offParent, offDefault, offString, offLen, offFloat, offInt; };

}  // namespace bm

#if defined(_WIN64)
#include "win64.h"
#elif defined(_WIN32)
#include "win32.h"
#elif defined(__linux__) && defined(__i386__)
#include "linux_x86.h"
#else
#error "bigmaps: no target data for this platform (supported: Windows x86/x64, Linux x86)"
#endif
