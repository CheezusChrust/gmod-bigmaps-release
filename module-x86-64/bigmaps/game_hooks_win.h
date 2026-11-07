// Game process mode (Windows only). Private to bigmaps.cpp (one translation unit).
//
// In the game the module is loaded once from the main-menu Lua state and stays for the session (pinned). Two
// engine hooks call back into the module before a map is loaded:
//   spawn: SV_SpawnServer(map, startspot): the host side (single-player, listen server, changelevel, map);
//   clnew: the client's signon-state-NEW step: joining a server, after downloads, before the map is loaded and
//          the SendTable CRC is sent.
// Each hook replaces the function's verified prologue (kHooks, relocation-free) with a jump to a stub that calls
// the module, then runs the original prologue bytes and jumps back behind them.
#pragma once

#include "patcher.h"
#include "client_hook_replay.h"

#include <initializer_list>

namespace
{

bool g_hooked = false;

// Checks the named map and switches the state to match; an unreadable map keeps the current state.
void SwitchForMap(const char* level, const char* via)
{
    char name[128], info[200] = "";
    NormalizeMapName(level, name, sizeof(name));
    MapSize size = CheckMap(g_fs, name, info, sizeof(info));
    Out("[bigmaps] %s: map '%s': %s; %s\n", via, name, info, SizeVerdict(size));
    if (size != MAP_UNKNOWN)
        SwitchState(size == MAP_LARGE);
}

void __cdecl OnSpawn(const char* map)
{
    if (map && Readable(map, 1))
        SwitchForMap(map, "host map load");
}

// `self` is the engine's client state object: kClStateOff = signon state, kClLevelOff = the level name
// (target data). Only signon state 3 (NEW) is acted on.
void __cdecl OnClientNew(const unsigned char* self)
{
    if (!self || !Readable(self + kClStateOff, 4) || !Readable(self + kClLevelOff, 1))
        return;
    int state;
    memcpy(&state, self + kClStateOff, 4);
    if (state != 3)
        return;
    char level[256];
    memcpy(level, self + kClLevelOff, sizeof(level) - 1);
    level[sizeof(level) - 1] = 0;
    SwitchForMap(level, "joining server");
}

// Appends machine code to a buffer.
struct Emitter
{
    unsigned char* p;
    void Bytes(std::initializer_list<unsigned char> b) { for (unsigned char c : b) *p++ = c; }
    void Raw(const void* src, size_t n) { memcpy(p, src, n); p += n; }
    template <typename T> void Value(T v) { memcpy(p, &v, sizeof(v)); p += sizeof(v); }
};

// Builds the stub for one hook at `stub`; returns its size. The stub preserves the hooked function's argument
// registers, calls `callback`, then runs the saved prologue and jumps back to target + prologue length.
// mapArg: the callback takes the map name (spawn: second register argument on x64 after `this`, first stack
// argument on x86); otherwise it takes `this` (rcx / ecx).
size_t BuildStub(unsigned char* stub, const ProbeSite& h, uintptr_t target, void* callback, bool mapArg)
{
    Emitter e{stub};
#ifdef _WIN64
    e.Bytes({0x51, 0x52, 0x41, 0x50, 0x41, 0x51});  // push rcx, rdx, r8, r9
    e.Bytes({0x48, 0x83, 0xEC, 0x28});              // sub rsp, 0x28 (shadow space, 16-byte aligned)
    if (mapArg)
        e.Bytes({0x48, 0x89, 0xD1});                // mov rcx, rdx
    e.Bytes({0x48, 0xB8});                          // mov rax, callback
    e.Value(reinterpret_cast<uint64_t>(callback));
    e.Bytes({0xFF, 0xD0});                          // call rax
    e.Bytes({0x48, 0x83, 0xC4, 0x28});              // add rsp, 0x28
    e.Bytes({0x41, 0x59, 0x41, 0x58, 0x5A, 0x59});  // pop r9, r8, rdx, rcx
    if (mapArg)
        e.Raw(h.stock, h.len);
    else
    {
        unsigned char replay[32];
        const size_t replaySize = BuildClientHookReplay(replay, h.stock, target - h.rva);
        e.Raw(replay, replaySize);
    }
    e.Bytes({0xFF, 0x25, 0, 0, 0, 0});              // jmp [rip+0]
    e.Value(static_cast<uint64_t>(target + h.len));
#else
    e.Bytes({0x51, 0x52});                          // push ecx; push edx
    if (mapArg)
        e.Bytes({0xFF, 0x74, 0x24, 0x0C});          // push dword [esp+0xC] (first stack argument)
    else
        e.Bytes({0x51});                            // push ecx (this)
    e.Bytes({0xB8});                                // mov eax, callback
    e.Value(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(callback)));
    e.Bytes({0xFF, 0xD0});                          // call eax
    e.Bytes({0x83, 0xC4, 0x04});                    // add esp, 4 (cdecl: caller pops the argument)
    e.Bytes({0x5A, 0x59});                          // pop edx; pop ecx
    e.Raw(h.stock, h.len);                          // original prologue
    e.Bytes({0xE9});                                // jmp rel32
    e.Value(static_cast<int32_t>(static_cast<intptr_t>(target + h.len) - static_cast<intptr_t>(
                                     reinterpret_cast<uintptr_t>(e.p) + 4)));
#endif
    return static_cast<size_t>(e.p - stub);
}

// Builds both stubs in one page (16-byte aligned, then made execute-read) and overwrites each prologue with a
// jump to its stub; the rest of the prologue is filled with int3. Hook prologues are at least as long as the
// jump (x64 14 bytes, x86 5 bytes).
bool InstallHooks(HMODULE eng)
{
    uintptr_t delta = LoadDelta("engine.dll", eng);
    for (const ProbeSite& h : kHooks)
        if (ClassifySite(h, eng, delta) != SITE_PATCHED)  // stock == patched for hook prologues
        {
            Out("[bigmaps] REFUSED: hook prologue '%s' does not match. Nothing was written\n", h.id);
            return false;
        }
    unsigned char* mem = static_cast<unsigned char*>(VirtualAlloc(nullptr, 4096, MEM_COMMIT | MEM_RESERVE,
                                                                  PAGE_READWRITE));
    if (!mem)
        return false;
    unsigned char* stubs[2] = {};
    size_t used = 0;
    for (int i = 0; i < 2; ++i)
    {
        const ProbeSite& h = kHooks[i];
        bool spawn = !strcmp(h.id, "spawn");
        stubs[i] = mem + used;
        used += (BuildStub(stubs[i], h, reinterpret_cast<uintptr_t>(eng) + h.rva,
                           spawn ? reinterpret_cast<void*>(&OnSpawn) : reinterpret_cast<void*>(&OnClientNew), spawn) +
                 15) & ~size_t(15);
    }
    DWORD old;
    if (!VirtualProtect(mem, 4096, PAGE_EXECUTE_READ, &old))
        return false;
    FlushInstructionCache(GetCurrentProcess(), mem, used);
    for (int i = 0; i < 2; ++i)
    {
        const ProbeSite& h = kHooks[i];
        unsigned char* at = reinterpret_cast<unsigned char*>(eng) + h.rva;
        unsigned char jmp[32];
        memset(jmp, 0xCC, sizeof(jmp));
#ifdef _WIN64
        jmp[0] = 0xFF; jmp[1] = 0x25; memset(jmp + 2, 0, 4);  // jmp [rip+0], 8-byte target follows
        uint64_t dest = reinterpret_cast<uint64_t>(stubs[i]);
        memcpy(jmp + 6, &dest, 8);
#else
        jmp[0] = 0xE9;  // jmp rel32
        int32_t rel = static_cast<int32_t>(reinterpret_cast<intptr_t>(stubs[i]) - reinterpret_cast<intptr_t>(at + 5));
        memcpy(jmp + 1, &rel, 4);
#endif
        if (!WriteBytes(at, jmp, h.len))
        {
            Out("[bigmaps] hook '%s': write failed (error %lu)\n", h.id, GetLastError());
            return false;
        }
        Out("[bigmaps] hook %s installed at %p\n", h.id, (void*)at);
    }
    return true;
}

// Verifies once, acquires the file system, pins this DLL and installs the hooks. The patch state itself only
// changes from the hooks, at the next map load.
void OpenGameMode()
{
    if (g_hooked)
    {
        Out("[bigmaps] gmod13_open: already active for this game session\n");
        return;
    }
    Plan plan;
    if (!Verify(plan, false))
        return;
    char why[200] = "";
    g_fs = GetFileSystem(why, sizeof(why));
    if (!g_fs)
    {
        Out("[bigmaps] REFUSED: %s. Nothing was written\n", why);
        return;
    }
    // The hooks call into this DLL for the rest of the session: keep it loaded even if Lua unloads it.
    HMODULE self = nullptr;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_PIN,
                       reinterpret_cast<LPCSTR>(&OnSpawn), &self);
    g_hooked = InstallHooks(plan.mod[MOD_ENGINE]);
    Out("[bigmaps] %s; currently %s\n", g_hooked ? "game mode active: switching at each map load" : "hooks NOT installed",
        plan.AllPatched() ? "patched" : "stock");
}

}  // namespace
