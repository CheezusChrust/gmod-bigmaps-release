// bigmaps: experimental in-memory patcher that doubles the usable map extents (+-16384 -> +-32768).
// Loaded by require("bigmaps") as gmsv_bigmaps_win32.dll / gmsv_bigmaps_win64.dll (game and dedicated server) or
// gmsv_bigmaps_linux.dll (Linux x86 dedicated server). The Lua side is only the require.
//
// Source layout (built as this one translation unit; see tools/build-module*):
//   ../targets/      per-build data: patch sites, hashes, RVAs and struct offsets (win64.h, win32.h, linux_x86.h)
//   ../common/       platform layer (platform_win.h / platform_linux.h) and shared helpers: identity and site
//                    checks, SendTable access, engine CRC / host state, map size check, console output
//   patcher.h        verify and switch the patch state (what is changed and why: see there)
//   game_hooks_win.h game process mode: engine hooks that switch before each map load (Windows only)
//
// Switching is automatic per map: a map with any world bound beyond +-16384 gets the patched state, any other map
// the stock one. When that happens depends on the process (see the entry points below).

#include "patcher.h"

namespace
{

bool g_verified = false;  // dedicated server: gmod13_open verified everything; consumed by gmod13_close
bool g_gameMode = false;  // loaded in the game (no dedicated-server module present)

}  // namespace

#ifdef _WIN32
#include "game_hooks_win.h"
#else
namespace
{

void OpenGameMode()
{
    Out("[bigmaps] REFUSED: %s not loaded; the Linux build supports the dedicated server only. Nothing was written\n",
        kDedicated);
}

}  // namespace
#endif

// Dedicated server (dedicated.dll / dedicated_srv.so loaded; server autorun require): only verifies and reports.
// The change itself happens in gmod13_close.
// Game process (main-menu require, Windows only): installs the two hooks once; see OpenGameMode.
BM_EXPORT int gmod13_open(void* /*lua_State*/)
{
    InitConsole();
    g_gameMode = GetModuleHandleA(kDedicated) == nullptr;
    Out("[bigmaps] gmod13_open: %s, experimental in-memory patcher (automatic per map, %s)\n",
#ifdef _WIN32
        sizeof(void*) == 8 ? "x64" : "x86",
#else
        "Linux x86",
#endif
        g_gameMode ? "game" : "dedicated server");
    if (g_gameMode)
    {
        OpenGameMode();
        return 0;
    }
    Plan plan;
    g_verified = Verify(plan, false);
    if (!g_verified)
        return 0;
    char why[200] = "";
    g_fs = GetFileSystem(why, sizeof(why));
    if (!g_fs)
    {
        Out("[bigmaps] REFUSED: %s. Nothing will be written\n", why);
        g_verified = false;
        return 0;
    }
    Out("[bigmaps] verified: sites stock=%d patched=%d; sendprops stock=%d patched=%d; SendTable CRC 0x%08X; %s\n",
        plan.sitesStock, plan.sitesPatched, plan.propsStock, plan.propsPatched, plan.crcStored,
        plan.AllPatched() ? "active" : "stock");
    return 0;
}

// Dedicated server: GMod calls this when the server's Lua state shuts down at a map change, before the next map's
// BSP is loaded and its extents are checked. Reads the pending map name from the engine's host state, checks that
// one BSP's bounds, and patches (large map) or restores stock (normal map). Server shutdown (quit) is not a map
// change: nothing is written. The module is required again by the next map's autorun.
BM_EXPORT int gmod13_close(void* /*lua_State*/)
{
    if (g_gameMode)
        return 0;  // the hooks stay for the whole game session
    bool verified = g_verified;
    g_verified = false;
    HMODULE eng = GetModuleHandleA(kModules[MOD_ENGINE]);
    HostStateView hs = {};
    if (!verified || !g_fs || !eng || !ReadHostState(eng, &hs))
    {
        Out("[bigmaps] gmod13_close: %s; nothing changed\n", verified ? "host state unreadable" : "not verified");
        return 0;
    }
    bool stopping = hs.next == HS_SHUTDOWN || hs.next == HS_QUIT || hs.next == HS_RESTART;
    bool mapChange = !stopping && (hs.next == HS_NEW_GAME || hs.next == HS_CHANGE_LEVEL ||
                                   hs.current == HS_NEW_GAME || hs.current == HS_CHANGE_LEVEL);
    if (!mapChange)
    {
        Out("[bigmaps] gmod13_close: host state %d -> %d, not a map change; nothing changed\n", hs.current, hs.next);
        return 0;
    }
    char name[128], info[200] = "";
    NormalizeMapName(hs.level, name, sizeof(name));
    MapSize size = CheckMap(g_fs, name, info, sizeof(info));
    Out("[bigmaps] next map '%s': %s; %s\n", name, info, SizeVerdict(size));
    if (size != MAP_UNKNOWN)
        SwitchState(size == MAP_LARGE);
    return 0;
}
