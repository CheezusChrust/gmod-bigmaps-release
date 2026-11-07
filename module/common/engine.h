// Engine internals reached through fixed RVAs from the target data, each guarded by kAnchors (unchanged engine
// code that refers to them): the server's SendTable CRC and the host state (pending map name).
#pragma once

#include "patch_sites.h"

namespace bm
{

// The engine computes the SendTable CRC once in SendTable_Init (before any module loads) and compares it with
// each joining client's. The anchors cover the getter, the CRC function and SendTable_Init's store.
inline bool AnchorsOk(HMODULE eng, uintptr_t delta, const char** bad)
{
    for (const ProbeSite& a : kAnchors)
        if (ClassifySite(a, eng, delta) != SITE_PATCHED)  // stock == patched for anchors
        {
            *bad = a.id;
            return false;
        }
    return true;
}

inline unsigned* CrcGlobal(HMODULE eng)
{
    return reinterpret_cast<unsigned*>(reinterpret_cast<uintptr_t>(eng) + kCrcGlobalRva);
}

// Calls the engine's own CRC function: a pure CRC32 over the registered SendTables, no arguments.
inline unsigned ComputeCrc(HMODULE eng)
{
    typedef unsigned(__cdecl * CrcFn)();
    return reinterpret_cast<CrcFn>(reinterpret_cast<uintptr_t>(eng) + kCrcFuncRva)();
}

// Engine host-state block (verified by the hs_name / hs_state anchors): +0 current state, +4 next state,
// +0x20 level name (0x100 bytes).
struct HostStateView { int current; int next; char level[256]; };

// Host states seen at a map change or shutdown (observed: 3 -> 3 on changelevel, 5 -> 6 on quit).
enum HostState { HS_NEW_GAME = 0, HS_CHANGE_LEVEL = 3, HS_SHUTDOWN = 5, HS_QUIT = 6, HS_RESTART = 7 };

inline bool ReadHostState(HMODULE eng, HostStateView* out)
{
    const unsigned char* hs = reinterpret_cast<const unsigned char*>(eng) + kHostStateRva;
    if (!Readable(hs, 0x120))
        return false;
    memcpy(&out->current, hs, 4);
    memcpy(&out->next, hs + 4, 4);
    memcpy(out->level, hs + 0x20, 255);
    out->level[255] = 0;
    return true;
}

}  // namespace bm
