// Experimental in-memory patcher (dedicated server, Windows x86/x64 and Linux x86;
// the game, Windows only). Loaded by `require("bigmaps")` as gmsv_bigmaps_win32/_win64/_linux.dll.
//
// Applies the expanded map limits to the running engine.dll / server.dll (and client.dll if loaded); on Linux
// engine_srv.so / server_srv.so:
//   1. code/data sites from module/common/sites.h (relocated values adjusted by the load delta);
//   2. the SendProp fields that the server's registration code already filled in before any module
//      could load: DT_Beam m_vecOrigin 19 -> 20 bits, DT_EffectData m_vOrigin/m_vStart 17 -> 18 bits,
//      bounds +-16384 -> +-32768 and the derived range multiplier;
//   3. the server's SendTable CRC (computed once at startup): recomputed with the engine's own CRC
//      function after step 2 so that clients with matching changes can connect (otherwise they are dropped with
//      "Server uses different class tables"). Before writing, that function must reproduce the stored CRC.
// Everything is verified before anything is written: each file must be the known stock build (file-patched
// copies are refused; the module is the only supported way), every site must read exactly stock or
// exactly patched, and every SendProp must hold the stock or the patched values. Any mismatch refuses the
// whole module (nothing written).
// Automatic per map: gmod13_open only verifies. At each map change gmod13_close reads the pending map name
// from the engine, reads that one BSP's world bounds through the engine's file system, and applies the
// patches for a map beyond +-16384 or restores the stock bytes and values for a normal one (see the entry
// points below). The Lua side is only `require("bigmaps")`.

#include "../common/bm_fs.h"

#include <initializer_list>

using namespace bm;

namespace {

#ifdef _WIN32
const char* const kModules[] = {"engine.dll", "server.dll", "client.dll"};
#else
const char* const kModules[] = {"engine_srv.so", "server_srv.so"};
#endif
const int kNumModules = sizeof(kModules) / sizeof(kModules[0]);

struct PropTarget { const char* table; const char* var; int stockBits; int newBits; };
const PropTarget kProps[] = {
    {"DT_Beam", "m_vecOrigin", 19, 20},
    {"DT_EffectData", "m_vOrigin[0]", 17, 18}, {"DT_EffectData", "m_vOrigin[1]", 17, 18},
    {"DT_EffectData", "m_vOrigin[2]", 17, 18}, {"DT_EffectData", "m_vStart[0]", 17, 18},
    {"DT_EffectData", "m_vStart[1]", 17, 18}, {"DT_EffectData", "m_vStart[2]", 17, 18},
};
const int kNumProps = sizeof(kProps) / sizeof(kProps[0]);

struct Plan
{
    HMODULE mod[3] = {};
    uintptr_t delta[3] = {};
    int sitesToWrite = 0, sitesDone = 0;
    uintptr_t prop[kNumProps] = {};
    bool propToWrite[kNumProps] = {};
    bool propPatched[kNumProps] = {};
    int propsToWrite = 0, propsDone = 0;
    unsigned crcStored = 0;
};

int ModIndex(const char* name)
{
    for (int i = 0; i < kNumModules; ++i)
        if (!strcmp(kModules[i], name))
            return i;
    return -1;
}

bool Refuse(bool quiet, const char* why)
{
    Out("[bigmaps] %s: %s.%s\n", quiet ? "RE-CHECK FAILED" : "REFUSED", why, quiet ? "" : " Nothing was written");
    return false;
}

bool Verify(Plan& plan, bool quiet)
{
    char msg[256];
    for (int i = 0; i < kNumModules; ++i)
    {
        HMODULE m = GetModuleHandleA(kModules[i]);
        if (!m)
            continue;
        char hex[65] = "";
        FileId id = IdentifyFile(kModules[i], m, hex);
        if (!quiet)
            Out("[bigmaps] %-10s base=%p file=%s\n", kModules[i], (void*)m, FileIdName(id));
        if (id != BMF_STOCK)
        {
            sprintf_s(msg, "%s is not the stock build this module supports (%s, sha256 %s)", kModules[i],
                      FileIdName(id), hex[0] ? hex : "unreadable");
            return Refuse(quiet, msg);
        }
        plan.mod[i] = m;
        plan.delta[i] = LoadDelta(kModules[i], m);
    }
    if (!plan.mod[0] || !plan.mod[1])
        return Refuse(quiet, "the engine or server module is not loaded");

    for (const ProbeSite& s : kSites)
    {
        int i = ModIndex(s.module);
        if (i < 0 || !plan.mod[i])
            continue;
        SiteState st = ClassifySite(s, plan.mod[i], plan.delta[i]);
        if (st == SITE_STOCK)
            ++plan.sitesToWrite;
        else if (st == SITE_PATCHED)
            ++plan.sitesDone;
        else
        {
            sprintf_s(msg, "%s site %s (rva 0x%08X) is %s", s.module, s.id, s.rva,
                      st == SITE_OTHER ? "neither stock nor patched" : "unreadable");
            return Refuse(quiet, msg);
        }
    }

    HMODULE srv = plan.mod[1];
    for (int k = 0; k < kNumProps; ++k)
    {
        const PropTarget& t = kProps[k];
        SendTableRef tbl = FindSendTable(srv, t.table);
        if (!tbl.table || tbl.matches != 1)
        {
            sprintf_s(msg, "SendTable %s found %d times", t.table, tbl.matches);
            return Refuse(quiet, msg);
        }
        uintptr_t p = FindProp(srv, tbl, t.var);
        if (!p)
        {
            sprintf_s(msg, "%s.%s not found", t.table, t.var);
            return Refuse(quiet, msg);
        }
        PropValues v = ReadProp(p);
        if (SameProp(v, RangeValues(t.stockBits, 16384.0f)))
        {
            plan.propToWrite[k] = true;
            ++plan.propsToWrite;
        }
        else if (SameProp(v, RangeValues(t.newBits, 32768.0f)))
        {
            plan.propPatched[k] = true;
            ++plan.propsDone;
        }
        else
        {
            sprintf_s(msg, "%s.%s holds unexpected values (bits %d, %.3f..%.3f, mul %.6f)", t.table, t.var, v.bits,
                      v.low, v.high, v.mul);
            return Refuse(quiet, msg);
        }
        plan.prop[k] = p;
    }

    const char* bad = "";
    if (!AnchorsOk(plan.mod[0], plan.delta[0], &bad))
    {
        sprintf_s(msg, "engine CRC anchor '%s' does not match", bad);
        return Refuse(quiet, msg);
    }
    plan.crcStored = *CrcGlobal(plan.mod[0]);
    unsigned now = ComputeCrc(plan.mod[0]);
    if (now != plan.crcStored)
    {
        sprintf_s(msg, "SendTable CRC recomputed as 0x%08X but the engine holds 0x%08X", now, plan.crcStored);
        return Refuse(quiet, msg);
    }
    return true;
}

#ifdef _WIN32
bool WriteBytes(void* at, const void* src, size_t n)
{
    DWORD cur = Protection(at) & 0xFF;
    bool exec = cur == PAGE_EXECUTE || cur == PAGE_EXECUTE_READ || cur == PAGE_EXECUTE_READWRITE ||
                cur == PAGE_EXECUTE_WRITECOPY;
    DWORD old;
    if (!VirtualProtect(at, n, exec ? PAGE_EXECUTE_READWRITE : PAGE_READWRITE, &old))
        return false;
    memcpy(at, src, n);
    DWORD tmp;
    VirtualProtect(at, n, old, &tmp);
    FlushInstructionCache(GetCurrentProcess(), at, n);
    return true;
}
#else
// Adds write access to each page touched (keeping its read/execute bits, so other threads can still run code
// there), copies, then restores each page's original protection from /proc/self/maps.
bool WriteBytes(void* at, const void* src, size_t n)
{
    const uintptr_t page = static_cast<uintptr_t>(sysconf(_SC_PAGESIZE));
    const uintptr_t first = reinterpret_cast<uintptr_t>(at) & ~(page - 1);
    const uintptr_t last = (reinterpret_cast<uintptr_t>(at) + n - 1) & ~(page - 1);
    int prot[2] = {-1, -1};
    if (last - first > page)
        return false;  // sites are at most 32 bytes
    RefreshMaps();
    bool ok = true;
    uintptr_t k = 0;
    for (uintptr_t p = first; ok && p <= last; p += page, ++k)
    {
        prot[k] = PageProt(p);
        ok = prot[k] >= 0 && ((prot[k] & PROT_WRITE) || !mprotect(reinterpret_cast<void*>(p), page, prot[k] | PROT_WRITE));
        if (!ok)
            prot[k] = -1;
    }
    if (ok)
        memcpy(at, src, n);
    for (uintptr_t p = first, j = 0; j < k; p += page, ++j)
        if (prot[j] >= 0 && !(prot[j] & PROT_WRITE))
            mprotect(reinterpret_cast<void*>(p), page, prot[j]);
    return ok;
}
#endif

// Writes one direction for every item not yet in that state: on = stock -> patched, off = patched -> stock.
// The SendTable CRC is recomputed with the engine's function whenever a SendProp changed.
int Apply(const Plan& plan, bool on)
{
    int failed = 0;
    const SiteState from = on ? SITE_STOCK : SITE_PATCHED;
    for (const ProbeSite& s : kSites)
    {
        int i = ModIndex(s.module);
        if (i < 0 || !plan.mod[i] || ClassifySite(s, plan.mod[i], plan.delta[i]) != from)
            continue;
        unsigned char ep[32];
        Expected(s, on ? s.patched : s.stock, plan.delta[i], ep);
        if (!WriteBytes(reinterpret_cast<unsigned char*>(plan.mod[i]) + s.rva, ep, s.len))
        {
            Out("[bigmaps] write failed: %s %s (error %lu)\n", s.module, s.id, BM_LASTERR());
            ++failed;
        }
    }
    int propsWritten = 0;
    for (int k = 0; k < kNumProps; ++k)
    {
        if (plan.propPatched[k] == on)
            continue;
        PropValues nv = on ? RangeValues(kProps[k].newBits, 32768.0f) : RangeValues(kProps[k].stockBits, 16384.0f);
        uintptr_t p = plan.prop[k];
        bool ok = WriteBytes(reinterpret_cast<void*>(p + kPropBits), &nv.bits, 4) &&
                  WriteBytes(reinterpret_cast<void*>(p + kPropLow), &nv.low, 4) &&
                  WriteBytes(reinterpret_cast<void*>(p + kPropHigh), &nv.high, 4) &&
                  WriteBytes(reinterpret_cast<void*>(p + kPropHLMul), &nv.mul, 4);
        if (!ok)
        {
            Out("[bigmaps] write failed: %s.%s (error %lu)\n", kProps[k].table, kProps[k].var, BM_LASTERR());
            ++failed;
        }
        ++propsWritten;
    }
    if (propsWritten)
    {
        unsigned crc = ComputeCrc(plan.mod[0]);
        if (WriteBytes(CrcGlobal(plan.mod[0]), &crc, 4))
            Out("[bigmaps] SendTable CRC 0x%08X -> 0x%08X\n", plan.crcStored, crc);
        else
        {
            Out("[bigmaps] write failed: SendTable CRC (error %lu)\n", BM_LASTERR());
            ++failed;
        }
    }
    return failed;
}

IFsBase* g_fs = nullptr;
bool g_verified = false;

#ifdef _WIN32
// r_mapextents (client.dll; P4-12 changes only its default string). The ConVar is registered at startup, so its
// current value was copied from the stock default before any module loaded; keep the live value in step with
// the patch state. Located from its registration code (x64 0x18003CF30, x86 0x1005B2B0) and the ConVar
// constructor's field stores; every pointer is checked before the in-place write ("16384" and "32768" have
// the same length).
struct MapExtentsVar { unsigned objRva, nameRva, defaultRva, offName, offParent, offDefault, offString, offLen, offFloat, offInt; };
#ifdef _WIN64
const MapExtentsVar kMapExtents = {0x00A36950, 0x006F00A8, 0x006F009C, 0x18, 0x38, 0x40, 0x48, 0x50, 0x54, 0x58};
#else
const MapExtentsVar kMapExtents = {0x007EB1E0, 0x005C8A6C, 0x005C8A64, 0x0C, 0x1C, 0x20, 0x24, 0x28, 0x2C, 0x30};
#endif

void SyncMapExtents(bool on)
{
    HMODULE m = GetModuleHandleA("client.dll");
    if (!m)
        return;
    const MapExtentsVar& v = kMapExtents;
    uintptr_t b = reinterpret_cast<uintptr_t>(m);
    unsigned char* obj = reinterpret_cast<unsigned char*>(b + v.objRva);
    if (!Readable(obj, v.offInt + 4))
        return;
    auto ptrAt = [&](unsigned off) { uintptr_t x; memcpy(&x, obj + off, sizeof(x)); return x; };
    char* str = reinterpret_cast<char*>(ptrAt(v.offString));
    int len;
    memcpy(&len, obj + v.offLen, 4);
    if (ptrAt(v.offName) != b + v.nameRva || ptrAt(v.offParent) != reinterpret_cast<uintptr_t>(obj) ||
        ptrAt(v.offDefault) != b + v.defaultRva || len != 6 || !str || !Readable(str, 6))
    {
        Out("[bigmaps] r_mapextents: unexpected ConVar layout; left unchanged\n");
        return;
    }
    const char* want = on ? "32768" : "16384";
    const char* other = on ? "16384" : "32768";
    if (!memcmp(str, want, 6))
        return;
    if (memcmp(str, other, 6))
    {
        Out("[bigmaps] r_mapextents: value '%.5s' was changed by hand; left unchanged\n", str);
        return;
    }
    float f = on ? 32768.0f : 16384.0f;
    int n = on ? 32768 : 16384;
    memcpy(str, want, 6);
    memcpy(obj + v.offFloat, &f, 4);
    memcpy(obj + v.offInt, &n, 4);
    Out("[bigmaps] r_mapextents: %s -> %s\n", other, want);
}
#else
void SyncMapExtents(bool) {}  // client-only ConVar; Linux builds are dedicated-server only
#endif

void ChangeNow(bool on)
{
    Plan plan;
    if (!Verify(plan, false))
        return;
    bool pending = on ? (plan.sitesToWrite || plan.propsToWrite) : (plan.sitesDone || plan.propsDone);
    if (!pending)
    {
        Out("[bigmaps] already %s: nothing to write\n", on ? "patched" : "stock");
        SyncMapExtents(on);
        return;
    }
    int sites = on ? plan.sitesToWrite : plan.sitesDone;
    int props = on ? plan.propsToWrite : plan.propsDone;
    int failed = Apply(plan, on);
    Plan after;
    bool ok = Verify(after, true) &&
              (on ? !after.sitesToWrite && !after.propsToWrite : !after.sitesDone && !after.propsDone);
    Out("[bigmaps] %s: wrote %d sites and %d sendprops, %d write failures; re-check %s\n",
        ok && !failed ? (on ? "APPLIED" : "UNLOADED") : "FAILED", sites, props, failed,
        ok ? (on ? "all patched" : "all stock") : "NOT complete");
    if (ok && !failed)
        SyncMapExtents(on);
}

// ---- Game process (client) mode: two engine hooks ----
// In the game the module is loaded once from the main-menu Lua state and stays for the session (pinned).
// Two hooks call back into the module before a map is loaded:
//   spawn: SV_SpawnServer(map, startspot): the host side (single-player, listen server, changelevel, map);
//   clnew: the client's signon-state-NEW step: joining a server, after downloads, before the map is loaded and
//          the SendTable CRC is sent.
// Each hook replaces the function's verified prologue (kHooks, relocation-free) with a jump to a stub that
// calls the module, then runs the original prologue bytes and jumps back behind them.
bool g_gameMode = false;
#ifdef _WIN32
bool g_hooked = false;

void Decide(const char* level, const char* via)
{
    char name[128], info[200] = "";
    NormalizeMapName(level, name, sizeof(name));
    MapSize size = CheckMap(g_fs, name, info, sizeof(info));
    Out("[bigmaps] %s: map '%s': %s; %s\n", via, name, info,
        size == MAP_LARGE ? "large, enabling" : size == MAP_NORMAL ? "normal, disabling" : "unknown, keeping the current state");
    if (size != MAP_UNKNOWN)
        ChangeNow(size == MAP_LARGE);
}

void __cdecl OnSpawn(const char* map)
{
    if (map && Readable(map, 1))
        Decide(map, "host map load");
}

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
    Decide(level, "joining server");
}

struct Emitter
{
    unsigned char* p;
    void Bytes(std::initializer_list<unsigned char> b) { for (unsigned char c : b) *p++ = c; }
    void Raw(const void* src, size_t n) { memcpy(p, src, n); p += n; }
    template <typename T> void Value(T v) { memcpy(p, &v, sizeof(v)); p += sizeof(v); }
};

// Builds the stub for one hook at `stub`; returns its size. `mapArg` = the callback takes the first stack /
// second register argument (spawn's map name); otherwise it takes `this`.
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
    e.Raw(h.stock, h.len);                          // original prologue
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
    e.Bytes({0x83, 0xC4, 0x04});                    // add esp, 4
    e.Bytes({0x5A, 0x59});                          // pop edx; pop ecx
    e.Raw(h.stock, h.len);                          // original prologue
    e.Bytes({0xE9});                                // jmp rel32
    e.Value(static_cast<int32_t>(static_cast<intptr_t>(target + h.len) - static_cast<intptr_t>(
                                     reinterpret_cast<uintptr_t>(e.p) + 4)));
#endif
    return static_cast<size_t>(e.p - stub);
}

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
        jmp[0] = 0xFF; jmp[1] = 0x25; memset(jmp + 2, 0, 4);  // jmp [rip+0]
        uint64_t dest = reinterpret_cast<uint64_t>(stubs[i]);
        memcpy(jmp + 6, &dest, 8);
#else
        jmp[0] = 0xE9;
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
    g_hooked = InstallHooks(plan.mod[0]);
    Out("[bigmaps] %s; currently %s\n", g_hooked ? "game mode active: switching at each map load" : "hooks NOT installed",
        !plan.sitesToWrite && !plan.propsToWrite ? "patched" : "stock");
}
#else
void OpenGameMode()
{
    Out("[bigmaps] REFUSED: %s not loaded; the Linux build supports the dedicated server only. Nothing was written\n",
        kDedicated);
}
#endif

}  // namespace

// Dedicated server (dedicated.dll / dedicated_srv.so loaded; server autorun require): gmod13_open only verifies
// and reports.
// Changes are written in gmod13_close, which GMod calls when the server's Lua state shuts down at a map change, before the next map's BSP is loaded and its extents are
// checked: the module reads the pending map name from the engine, checks that one BSP's bounds, and patches
// (large map) or restores stock (normal map). Server shutdown (quit) is not a map change: nothing is written.
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
    bool patched = !plan.sitesToWrite && !plan.propsToWrite;
    Out("[bigmaps] verified: sites stock=%d patched=%d; sendprops stock=%d patched=%d; SendTable CRC 0x%08X; %s\n",
        plan.sitesToWrite, plan.sitesDone, plan.propsToWrite, plan.propsDone, plan.crcStored,
        patched ? "active" : "stock");
    return 0;
}

BM_EXPORT int gmod13_close(void* /*lua_State*/)
{
    if (g_gameMode)
        return 0;  // the hooks stay for the whole game session
    bool verified = g_verified;
    g_verified = false;
    HMODULE eng = GetModuleHandleA(kModules[0]);
    HostStateView hs = {};
    if (!verified || !g_fs || !eng || !ReadHostState(eng, &hs))
    {
        Out("[bigmaps] gmod13_close: %s; nothing changed\n", verified ? "host state unreadable" : "not verified");
        return 0;
    }
    // next 0 = new game (`map`), 3 = `changelevel`; 5..7 = shutdown/quit/restart. Observed: 3 -> 3 on
    // changelevel, 5 -> 6 on quit.
    bool stopping = hs.next == 5 || hs.next == 6 || hs.next == 7;
    bool mapChange = !stopping && (hs.next == 0 || hs.next == 3 || hs.current == 0 || hs.current == 3);
    if (!mapChange)
    {
        Out("[bigmaps] gmod13_close: host state %d -> %d, not a map change; nothing changed\n", hs.current, hs.next);
        return 0;
    }
    char name[128], info[200] = "";
    NormalizeMapName(hs.level, name, sizeof(name));
    MapSize size = CheckMap(g_fs, name, info, sizeof(info));
    Out("[bigmaps] next map '%s': %s; %s\n", name, info,
        size == MAP_LARGE ? "large, enabling" : size == MAP_NORMAL ? "normal, disabling" : "unknown, keeping the current state");
    if (size != MAP_UNKNOWN)
        ChangeNow(size == MAP_LARGE);
    return 0;
}
