// Experimental Windows x64 game module for Steam x86-64 branch build 26439081.

#include "patcher.h"

namespace
{

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

// Main-menu require installs the game hooks once; dedicated-server use is refused.
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
    if (!g_gameMode)
    {
        Out("[bigmaps] REFUSED: this x86-64 branch build supports the Windows game only. Nothing was written\n");
        return 0;
    }
    OpenGameMode();
    return 0;
}

BM_EXPORT int gmod13_close(void* /*lua_State*/)
{
    return 0;
}
