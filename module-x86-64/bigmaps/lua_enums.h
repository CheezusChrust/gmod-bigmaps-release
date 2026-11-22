// Lua enum hooks. Private to bigmaps.cpp (one translation unit).
//
// server.dll / client.dll (Linux: server_srv.so) each have one function that fills a new Lua state's global table
// with the Source enums (MAX_EDICT_BITS, MAX_PLAYER_BITS, ...; GMod's LuaGameEnums::InitLibraries). It runs when a
// realm's Lua state is created, before any addon code, so a constant set at its end cannot have been read or
// localized by an addon earlier. The function is hooked at its entry (kEnumHooks, relocation-free prologue): the
// stub calls the original through a trampoline, then calls SetMemberDouble on the same global-table object for
// MIN_COORD_FLOAT and MAX_COORD_FLOAT with the current g_coord values (+-16384 stock, +-32768 large; see
// SetCoordLimits). The values are written after GMod's own, so a GMod that defines them itself is overridden the
// same way.
//
// The module's other hooks run before a map is loaded, so g_coord already matches the map when the realm's Lua
// state is created. On a dedicated server the module is first required from the server's own Lua state, after
// that state's enum function has run: the hook then applies from the next map's state.
#pragma once

#include "../common/emitter.h"
#include "patcher.h"

namespace
{

const char kMinCoordName[] = "MIN_COORD_FLOAT";
const char kMaxCoordName[] = "MAX_COORD_FLOAT";
const int kNumEnumHooks = sizeof(kEnumHooks) / sizeof(kEnumHooks[0]);
bool g_enumHooked = false;

// Builds the stub for one enum hook at `stub`; returns its size. `tramp` runs the original function (its
// prologue, then a jump behind it).
//   Windows x64: entry (rcx = this, rdx = lua), returns normally; rbx/rsi are saved around the calls.
//   Windows x86: stdcall (lua) with `ret 4`; the original pops the argument pushed here.
//   Linux x86:   cdecl (this, lua); the stub keeps its own 16-byte aligned frame for the argument slots.
size_t BuildEnumStub(unsigned char* stub, const unsigned char* tramp)
{
    Emitter e{stub};
    const char* const names[2] = {kMinCoordName, kMaxCoordName};
#if defined(_WIN64)
    e.Bytes({0x53, 0x56});                          // push rbx; push rsi
    e.Bytes({0x48, 0x83, 0xEC, 0x28});              // sub rsp, 0x28 (shadow space, 16-byte aligned)
    e.Bytes({0x48, 0x89, 0xD3});                    // mov rbx, rdx (lua)
    e.Bytes({0x48, 0xB8});                          // mov rax, tramp
    e.Value(reinterpret_cast<uint64_t>(tramp));
    e.Bytes({0xFF, 0xD0});                          // call rax (original; rcx and rdx untouched)
    e.Bytes({0x48, 0x89, 0xD9});                    // mov rcx, rbx
    e.Bytes({0x48, 0x8B, 0x01});                    // mov rax, [rcx]
    e.Bytes({0xFF, 0x90});                          // call [rax + kLuaGlobalSlot]
    e.Value(kLuaGlobalSlot);
    e.Bytes({0x48, 0x89, 0xC6});                    // mov rsi, rax (global-table object)
    for (int i = 0; i < 2; ++i)
    {
        e.Bytes({0x48, 0x89, 0xF1});                // mov rcx, rsi
        e.Bytes({0x48, 0xBA});                      // mov rdx, name
        e.Value(reinterpret_cast<uint64_t>(names[i]));
        e.Bytes({0x49, 0xB8});                      // mov r8, &g_coord[i]
        e.Value(reinterpret_cast<uint64_t>(&g_coord[i]));
        e.Bytes({0xF2, 0x41, 0x0F, 0x10, 0x10});    // movsd xmm2, [r8]
        e.Bytes({0x48, 0x8B, 0x06});                // mov rax, [rsi]
        e.Bytes({0xFF, 0x90});                      // call [rax + kLuaSetDoubleSlot]
        e.Value(kLuaSetDoubleSlot);
    }
    e.Bytes({0x48, 0x83, 0xC4, 0x28});              // add rsp, 0x28
    e.Bytes({0x5E, 0x5B, 0xC3});                    // pop rsi; pop rbx; ret
#elif defined(_WIN32)
    e.Bytes({0x53, 0x56});                          // push ebx; push esi
    e.Bytes({0x8B, 0x5C, 0x24, 0x0C});              // mov ebx, [esp+0xC] (lua)
    e.Bytes({0x53});                                // push ebx
    e.Bytes({0xB8});                                // mov eax, tramp
    e.Value(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(tramp)));
    e.Bytes({0xFF, 0xD0});                          // call eax (original; stdcall pops the argument)
    e.Bytes({0x89, 0xD9});                          // mov ecx, ebx
    e.Bytes({0x8B, 0x01});                          // mov eax, [ecx]
    e.Bytes({0xFF, 0x90});                          // call [eax + kLuaGlobalSlot] (thiscall, no arguments)
    e.Value(kLuaGlobalSlot);
    e.Bytes({0x89, 0xC6});                          // mov esi, eax (global-table object)
    for (int i = 0; i < 2; ++i)
    {
        e.Bytes({0xBA});                            // mov edx, &g_coord[i]
        e.Value(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&g_coord[i])));
        e.Bytes({0xFF, 0x72, 0x04});                // push dword [edx+4] (double, high half)
        e.Bytes({0xFF, 0x32});                      // push dword [edx]   (double, low half)
        e.Bytes({0x68});                            // push name
        e.Value(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(names[i])));
        e.Bytes({0x89, 0xF1});                      // mov ecx, esi
        e.Bytes({0x8B, 0x06});                      // mov eax, [esi]
        e.Bytes({0xFF, 0x90});                      // call [eax + kLuaSetDoubleSlot] (thiscall; callee pops)
        e.Value(kLuaSetDoubleSlot);
    }
    e.Bytes({0x5E, 0x5B, 0xC2, 0x04, 0x00});        // pop esi; pop ebx; ret 4
#else
    // Entry esp = 12 mod 16 (return address on a 16-byte aligned frame); after two pushes and sub 0x14 the stack
    // is 16-byte aligned at each call, and the original sees the same alignment as from its real caller. The
    // 0x14-byte frame holds this, lua / name, double for the cdecl calls. Entry arguments: this at [esp+0x20],
    // lua at [esp+0x24] once the frame is built.
    e.Bytes({0x53, 0x56});                          // push ebx; push esi
    e.Bytes({0x83, 0xEC, 0x14});                    // sub esp, 0x14
    e.Bytes({0x8B, 0x44, 0x24, 0x20});              // mov eax, [esp+0x20] (this)
    e.Bytes({0x89, 0x04, 0x24});                    // mov [esp], eax
    e.Bytes({0x8B, 0x5C, 0x24, 0x24});              // mov ebx, [esp+0x24] (lua)
    e.Bytes({0x89, 0x5C, 0x24, 0x04});              // mov [esp+4], ebx
    e.Bytes({0xB8});                                // mov eax, tramp
    e.Value(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(tramp)));
    e.Bytes({0xFF, 0xD0});                          // call eax (original; cdecl, caller cleans)
    e.Bytes({0x8B, 0x03});                          // mov eax, [ebx]
    e.Bytes({0x89, 0x1C, 0x24});                    // mov [esp], ebx
    e.Bytes({0xFF, 0x90});                          // call [eax + kLuaGlobalSlot]
    e.Value(kLuaGlobalSlot);
    e.Bytes({0x89, 0xC6});                          // mov esi, eax (global-table object)
    for (int i = 0; i < 2; ++i)
    {
        e.Bytes({0x89, 0x34, 0x24});                // mov [esp], esi
        e.Bytes({0xC7, 0x44, 0x24, 0x04});          // mov dword [esp+4], name
        e.Value(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(names[i])));
        e.Bytes({0xBA});                            // mov edx, &g_coord[i]
        e.Value(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&g_coord[i])));
        e.Bytes({0x8B, 0x02});                      // mov eax, [edx]
        e.Bytes({0x89, 0x44, 0x24, 0x08});          // mov [esp+8], eax (double, low half)
        e.Bytes({0x8B, 0x42, 0x04});                // mov eax, [edx+4]
        e.Bytes({0x89, 0x44, 0x24, 0x0C});          // mov [esp+0xC], eax (double, high half)
        e.Bytes({0x8B, 0x06});                      // mov eax, [esi]
        e.Bytes({0xFF, 0x90});                      // call [eax + kLuaSetDoubleSlot]
        e.Value(kLuaSetDoubleSlot);
    }
    e.Bytes({0x83, 0xC4, 0x14});                    // add esp, 0x14
    e.Bytes({0x5E, 0x5B, 0xC3});                    // pop esi; pop ebx; ret
#endif
    return static_cast<size_t>(e.p - stub);
}

// Executable memory for the trampolines and stubs: written while read/write, then switched to read/execute.
unsigned char* AllocStubPage()
{
#ifdef _WIN32
    return static_cast<unsigned char*>(VirtualAlloc(nullptr, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
#else
    void* p = mmap(nullptr, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    return p == MAP_FAILED ? nullptr : static_cast<unsigned char*>(p);
#endif
}

bool SealStubPage(unsigned char* mem, size_t used)
{
#ifdef _WIN32
    DWORD old;
    if (!VirtualProtect(mem, 4096, PAGE_EXECUTE_READ, &old))
        return false;
    FlushInstructionCache(GetCurrentProcess(), mem, used);
    return true;
#else
    (void)used;
    return mprotect(mem, 4096, PROT_READ | PROT_EXEC) == 0;
#endif
}

// The stubs call into this DLL (g_coord, the names): keep it loaded for the rest of the process.
void PinSelf()
{
#ifdef _WIN32
    HMODULE self = nullptr;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_PIN,
                       reinterpret_cast<LPCSTR>(&g_coord), &self);
#else
    Dl_info info;
    if (dladdr(reinterpret_cast<void*>(&g_coord), &info) && info.dli_fname)
        dlopen(info.dli_fname, RTLD_LAZY | RTLD_NODELETE);
#endif
}

// Verifies the prologue of every loaded module's enum function (Windows: server.dll and client.dll, client.dll is
// absent on a dedicated server; Linux: server_srv.so), then builds trampolines and stubs in one page and
// overwrites each prologue with a jump to its stub. Nothing is written unless every loaded prologue matches.
bool InstallEnumHooks()
{
    if (g_enumHooked)
        return true;
    HMODULE mods[kNumEnumHooks] = {};
    int found = 0;
    for (int i = 0; i < kNumEnumHooks; ++i)
    {
        const ProbeSite& h = kEnumHooks[i];
        mods[i] = GetModuleHandleA(h.module);
        if (!mods[i])
        {
            Out("[bigmaps] enum hook: %s not loaded; skipped\n", h.module);
            continue;
        }
        if (ClassifySite(h, mods[i], LoadDelta(h.module, mods[i])) != SITE_PATCHED)  // stock == patched
        {
            Out("[bigmaps] REFUSED: enum hook prologue in %s does not match. Nothing was written\n", h.module);
            return false;
        }
        ++found;
    }
    if (!found)
        return false;
    unsigned char* mem = AllocStubPage();
    if (!mem)
        return false;
    PinSelf();
    unsigned char* stubs[kNumEnumHooks] = {};
    size_t used = 0;
    for (int i = 0; i < kNumEnumHooks; ++i)
    {
        if (!mods[i])
            continue;
        const ProbeSite& h = kEnumHooks[i];
        uintptr_t target = reinterpret_cast<uintptr_t>(mods[i]) + h.rva;
        unsigned char* tramp = mem + used;
        Emitter t{tramp};
        t.Raw(h.stock, h.len);                      // original prologue
#ifdef _WIN64
        t.Bytes({0xFF, 0x25, 0, 0, 0, 0});          // jmp [rip+0]
        t.Value(static_cast<uint64_t>(target + h.len));
#else
        t.Bytes({0xE9});                            // jmp rel32
        t.Value(static_cast<int32_t>(static_cast<intptr_t>(target + h.len) -
                                     static_cast<intptr_t>(reinterpret_cast<uintptr_t>(t.p) + 4)));
#endif
        used += (static_cast<size_t>(t.p - tramp) + 15) & ~size_t(15);
        stubs[i] = mem + used;
        used += (BuildEnumStub(stubs[i], tramp) + 15) & ~size_t(15);
    }
    if (!SealStubPage(mem, used))
        return false;
    for (int i = 0; i < kNumEnumHooks; ++i)
    {
        if (!mods[i])
            continue;
        const ProbeSite& h = kEnumHooks[i];
        unsigned char* at = reinterpret_cast<unsigned char*>(mods[i]) + h.rva;
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
            Out("[bigmaps] enum hook %s: write failed (error %lu)\n", h.module, BM_LASTERR());
            return false;
        }
        Out("[bigmaps] enum hook installed in %s at %p (MIN/MAX_COORD_FLOAT = %.0f/%.0f)\n", h.module, (void*)at,
            g_coord[0], g_coord[1]);
    }
    g_enumHooked = true;
    return true;
}

}  // namespace
