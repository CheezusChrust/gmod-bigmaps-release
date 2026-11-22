// Ghost-injection mode (dedicated servers, Windows and Linux). Private to bigmaps.cpp (one translation unit).
//
// With `-usegh` the dedicated launcher (dedicated.dll / dedicated_srv.so) loads "ghostinj.dll" from the server's
// main directory before the engine is loaded, and calls nothing in it by name: only the library's entry runs
// (DllMain DLL_PROCESS_ATTACH on Windows, static constructors on Linux). HolyLib's own ghostinj.dll loads its
// holylib library and then a "ghostinj2.dll" the same way. Name this module ghostinj.dll (or ghostinj2.dll next
// to HolyLib's) and GhostLoad starts here. That is early enough for the FIRST map to be an oversized one: the
// `require("bigmaps")` route only starts from the first map's Lua state.
//
// At that time the engine and server modules are not loaded yet, so nothing can be verified or patched. A watcher
// thread waits until both appear (the engine loads the server module during its own start-up, long before the
// first map is spawned), identifies both files by hash, verifies every hook prologue, and then installs:
//   - the Lua enum hook (lua_enums.h): MIN/MAX_COORD_FLOAT in every Lua state, including the first;
//   - the map spawn hook, which switches the patch state before every map load (first map, changelevel, map): on
//     Windows the existing `spawn`/`clnew` engine hooks (game_hooks_win.h), on Linux a hook on
//     CGameServer::SpawnServer (kSpawnHook). SwitchState verifies everything again (SendProps, SendTable CRC) once
//     the engine has finished initializing.
// Nothing is written unless the files are the known stock build. Every later map change goes through the same
// spawn hook, so the autorun `require("bigmaps")` is not needed; if it is present, that second copy of the module
// sees kGhostEnv and does nothing.
#pragma once

#include "../common/emitter.h"
#include "patcher.h"

#ifdef _WIN32
#include <process.h>
#else
#include <pthread.h>
#endif

namespace
{

const char kGhostEnv[] = "BIGMAPS_GHOST";

// Set by the ghost copy; read by the copy that Lua `require`s (a separate DLL file with its own CRT environment,
// so the process environment is used).
bool GhostActive()
{
#ifdef _WIN32
    return GetEnvironmentVariableA(kGhostEnv, nullptr, 0) != 0;
#else
    return getenv(kGhostEnv) != nullptr;
#endif
}

void SetGhostActive()
{
#ifdef _WIN32
    SetEnvironmentVariableA(kGhostEnv, "1");
#else
    setenv(kGhostEnv, "1", 1);
#endif
}

#ifndef _WIN32
// CGameServer::SpawnServer callback: `map` is the map name (second stack argument of the hooked function).
void OnGhostSpawn(const char* map)
{
    if (map && Readable(map, 1))
        SwitchForMap(map, "host map load");
}

// Stub for the spawn hook: calls OnGhostSpawn(mapname) with an aligned frame, then runs the saved prologue and
// jumps back behind it. cdecl at function entry: [esp] = return address, [esp+4] = this, [esp+8] = mapname; no
// register is live, so none needs saving.
size_t BuildSpawnStub(unsigned char* stub, const ProbeSite& h, uintptr_t target)
{
    Emitter e{stub};
    e.Bytes({0x83, 0xEC, 0x1C});                    // sub esp, 0x1C (entry esp = 12 mod 16 -> aligned)
    e.Bytes({0x8B, 0x44, 0x24, 0x24});              // mov eax, [esp+0x24] (mapname: entry esp+8)
    e.Bytes({0x89, 0x04, 0x24});                    // mov [esp], eax
    e.Bytes({0xB8});                                // mov eax, OnGhostSpawn
    e.Value(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&OnGhostSpawn)));
    e.Bytes({0xFF, 0xD0});                          // call eax
    e.Bytes({0x83, 0xC4, 0x1C});                    // add esp, 0x1C
    e.Raw(h.stock, h.len);                          // original prologue
    e.Bytes({0xE9});                                // jmp rel32
    e.Value(static_cast<int32_t>(static_cast<intptr_t>(target + h.len) - static_cast<intptr_t>(
                                     reinterpret_cast<uintptr_t>(e.p) + 4)));
    return static_cast<size_t>(e.p - stub);
}

bool InstallSpawnHook(HMODULE eng)
{
    const ProbeSite& h = kSpawnHook;
    uintptr_t target = reinterpret_cast<uintptr_t>(eng) + h.rva;
    unsigned char* mem = AllocStubPage();
    if (!mem)
        return false;
    size_t used = BuildSpawnStub(mem, h, target);
    if (!SealStubPage(mem, used))
        return false;
    unsigned char jmp[32];
    memset(jmp, 0xCC, sizeof(jmp));
    jmp[0] = 0xE9;  // jmp rel32
    int32_t rel = static_cast<int32_t>(reinterpret_cast<intptr_t>(mem) - static_cast<intptr_t>(target + 5));
    memcpy(jmp + 1, &rel, 4);
    if (!WriteBytes(reinterpret_cast<void*>(target), jmp, h.len))
    {
        Out("[bigmaps] spawn hook: write failed (error %lu)\n", BM_LASTERR());
        return false;
    }
    Out("[bigmaps] spawn hook installed in %s at %p\n", h.module, reinterpret_cast<void*>(target));
    return true;
}
#endif

// Every hook prologue (engine spawn hooks, enum hooks of the loaded modules) must match before anything is written.
bool GhostPrologues(HMODULE eng)
{
    bool ok = true;
#ifdef _WIN32
    for (const ProbeSite& h : kHooks)
        if (ClassifySite(h, eng, LoadDelta(h.module, eng)) != SITE_PATCHED)  // stock == patched
        {
            Out("[bigmaps] REFUSED: hook prologue '%s' does not match. Nothing was written\n", h.id);
            ok = false;
        }
#else
    if (ClassifySite(kSpawnHook, eng, LoadDelta(kSpawnHook.module, eng)) != SITE_PATCHED)
    {
        Out("[bigmaps] REFUSED: spawn hook prologue in %s does not match. Nothing was written\n", kSpawnHook.module);
        ok = false;
    }
#endif
    for (const ProbeSite& h : kEnumHooks)
        if (HMODULE m = GetModuleHandleA(h.module))
            if (ClassifySite(h, m, LoadDelta(h.module, m)) != SITE_PATCHED)
            {
                Out("[bigmaps] REFUSED: enum hook prologue in %s does not match. Nothing was written\n", h.module);
                ok = false;
            }
    return ok;
}

enum GhostResult { GHOST_RETRY, GHOST_DONE, GHOST_REFUSED };

// One attempt, made once both modules are loaded: both must be the known stock files (hashed once; a file that
// cannot be read yet is retried) and every hook prologue must match; then the hooks are installed.
GhostResult GhostAttempt()
{
    static bool identified = false;
    static bool console = false;
    HMODULE eng = GetModuleHandleA(kModules[MOD_ENGINE]);
    HMODULE srv = GetModuleHandleA(kModules[MOD_SERVER]);
    if (!eng || !srv)
        return GHOST_RETRY;
    if (!console)  // tier0 is certainly loaded now; the entry point may have run before it
    {
        InitConsole();
        console = true;
    }
    if (!identified)
    {
        HMODULE mods[2] = {eng, srv};
        for (int i = 0; i < 2; ++i)
        {
            char hex[65] = "";
            FileId id = IdentifyFile(kModules[i], mods[i], hex);
            if (id == BMF_UNREADABLE)
                return GHOST_RETRY;
            if (id != BMF_STOCK)
            {
                Out("[bigmaps] REFUSED: %s is not the stock build this module supports (%s, sha256 %s). Nothing was "
                    "written\n", kModules[i], FileIdName(id), hex);
                return GHOST_REFUSED;
            }
        }
        identified = true;
    }
    if (!GhostPrologues(eng))
        return GHOST_REFUSED;
    if (!InstallEnumHooks())
        return GHOST_REFUSED;
#ifdef _WIN32
    g_hooked = InstallHooks(eng);
    return g_hooked ? GHOST_DONE : GHOST_REFUSED;
#else
    return InstallSpawnHook(eng) ? GHOST_DONE : GHOST_REFUSED;
#endif
}

// Watches for the engine and server modules for up to about a minute, then makes one attempt.
#ifdef _WIN32
unsigned __stdcall GhostThread(void*)
#else
void* GhostThread(void*)
#endif
{
    for (int i = 0; i < 20000; ++i)
    {
#ifdef _WIN32
        Sleep(3);
#else
        usleep(3000);
#endif
        GhostResult r = GhostAttempt();
        if (r == GHOST_RETRY)
            continue;
        Out("[bigmaps] ghostinj mode %s\n", r == GHOST_DONE ? "active: switching before every map load"
                                                              : "NOT active");
        return 0;
    }
    Out("[bigmaps] ghostinj mode: the engine and server modules were not loaded in time; not active\n");
    return 0;
}

// Entry point (see GhostMain below): when the launcher loads this file as ghostinj.dll the engine is not loaded
// yet; when Lua `require`s it the engine is already there and nothing happens.
void GhostLoad()
{
    if (GetModuleHandleA(kModules[MOD_ENGINE]))
        return;
    SetGhostActive();
    InitConsole();
    Out("[bigmaps] ghostinj: loaded before the engine (experimental in-memory patcher, %s dedicated server)\n",
#ifdef _WIN32
        sizeof(void*) == 8 ? "Windows x64" : "Windows x86");
#else
        "Linux x86");
#endif
    PinSelf();  // the stubs and the watcher thread live in this file
#ifdef _WIN32
    HANDLE t = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, GhostThread, nullptr, 0, nullptr));
    if (t)
        CloseHandle(t);
#else
    pthread_t t;
    bool started = pthread_create(&t, nullptr, GhostThread, nullptr) == 0;
    if (started)
        pthread_detach(t);
#endif
#ifdef _WIN32
    if (!t)
#else
    if (!started)
#endif
        Out("[bigmaps] ghostinj: could not start the watcher thread; not active\n");
}

}  // namespace

#ifdef _WIN32
// The dedicated launcher LoadLibrary()s ghostinj.dll: DLL_PROCESS_ATTACH is the entry point. The loader lock is
// held here, so GhostLoad only starts a thread and returns.
extern "C" BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
        GhostLoad();
    return TRUE;
}
#else
namespace
{
struct GhostStarter { GhostStarter() { GhostLoad(); } };
GhostStarter g_ghostStarter;  // static constructor: runs when the launcher dlopens this file
}  // namespace
#endif
