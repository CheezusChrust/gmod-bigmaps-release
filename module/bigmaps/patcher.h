// Verifying and switching the patch state. Private to bigmaps.cpp (one translation unit).
//
// The state is all or nothing across three kinds of change:
//   1. code/data sites (kSites in the target data), relocated values adjusted by the load delta;
//   2. SendProp fields that the server's registration code filled in before any module could load:
//      DT_Beam m_vecOrigin 19 -> 20 bits, DT_EffectData m_vOrigin/m_vStart 17 -> 18 bits, bounds
//      +-16384 -> +-32768 and the derived range multiplier;
//   3. the server's SendTable CRC (computed once at startup), recomputed with the engine's own CRC function after
//      step 2 so that clients with the same changes can connect (otherwise they are dropped with "Server uses
//      different class tables"). Before anything is written, that function must reproduce the stored CRC.
// Everything is verified before anything is written: each module must be the known stock build (file-patched
// copies are refused), every site must read exactly stock or exactly patched, and every SendProp must hold the
// stock or the patched values. Any mismatch refuses the whole change.
#pragma once

#include "../common/engine.h"
#include "../common/log.h"
#include "../common/map_check.h"
#include "../common/sendtable.h"

namespace
{

using namespace bm;

// The engine file system used for map checks; set once everything verified.
IFsBase* g_fs = nullptr;

// Patched modules; Plan::mod/delta are indexed in this order. client.dll is optional (game only).
#ifdef _WIN32
const char* const kModules[] = {"engine.dll", "server.dll", "client.dll"};
#else
const char* const kModules[] = {"engine_srv.so", "server_srv.so"};
#endif
const int kNumModules = sizeof(kModules) / sizeof(kModules[0]);
enum { MOD_ENGINE = 0, MOD_SERVER = 1, MOD_SLOTS = 3 };

struct PropTarget { const char* table; const char* var; int stockBits; int newBits; };
const PropTarget kProps[] = {
    {"DT_Beam", "m_vecOrigin", 19, 20},
    {"DT_EffectData", "m_vOrigin[0]", 17, 18}, {"DT_EffectData", "m_vOrigin[1]", 17, 18},
    {"DT_EffectData", "m_vOrigin[2]", 17, 18}, {"DT_EffectData", "m_vStart[0]", 17, 18},
    {"DT_EffectData", "m_vStart[1]", 17, 18}, {"DT_EffectData", "m_vStart[2]", 17, 18},
};
const int kNumProps = sizeof(kProps) / sizeof(kProps[0]);

PropValues StockProp(const PropTarget& t) { return RangeValues(t.stockBits, kStockExtent); }
PropValues LargeProp(const PropTarget& t) { return RangeValues(t.newBits, kLargeExtent); }

// What Verify found. "Stock" items still need writing to enable; "patched" items to disable.
struct Plan
{
    HMODULE mod[MOD_SLOTS] = {};
    uintptr_t delta[MOD_SLOTS] = {};
    int sitesStock = 0, sitesPatched = 0;
    uintptr_t prop[kNumProps] = {};
    bool propStock[kNumProps] = {};
    bool propPatched[kNumProps] = {};
    int propsStock = 0, propsPatched = 0;
    unsigned crcStored = 0;

    bool AllPatched() const { return !sitesStock && !propsStock; }
    bool AllStock() const { return !sitesPatched && !propsPatched; }
};

int ModIndex(const char* name)
{
    for (int i = 0; i < kNumModules; ++i)
        if (!strcmp(kModules[i], name))
            return i;
    return -1;
}

// quiet = the re-check after writing: no per-module report, and nothing is claimed about writes.
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
    if (!plan.mod[MOD_ENGINE] || !plan.mod[MOD_SERVER])
        return Refuse(quiet, "the engine or server module is not loaded");

    // Sites in modules that are not loaded (client.dll on a dedicated server) are skipped.
    for (const ProbeSite& s : kSites)
    {
        int i = ModIndex(s.module);
        if (i < 0 || !plan.mod[i])
            continue;
        SiteState st = ClassifySite(s, plan.mod[i], plan.delta[i]);
        if (st == SITE_STOCK)
            ++plan.sitesStock;
        else if (st == SITE_PATCHED)
            ++plan.sitesPatched;
        else
        {
            sprintf_s(msg, "%s site %s (rva 0x%08X) is %s", s.module, s.id, s.rva,
                      st == SITE_OTHER ? "neither stock nor patched" : "unreadable");
            return Refuse(quiet, msg);
        }
    }

    HMODULE srv = plan.mod[MOD_SERVER];
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
        if (SameProp(v, StockProp(t)))
        {
            plan.propStock[k] = true;
            ++plan.propsStock;
        }
        else if (SameProp(v, LargeProp(t)))
        {
            plan.propPatched[k] = true;
            ++plan.propsPatched;
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
    if (!AnchorsOk(plan.mod[MOD_ENGINE], plan.delta[MOD_ENGINE], &bad))
    {
        sprintf_s(msg, "engine CRC anchor '%s' does not match", bad);
        return Refuse(quiet, msg);
    }
    // Proves the CRC function and global are what we think: recomputing must give the stored value.
    plan.crcStored = *CrcGlobal(plan.mod[MOD_ENGINE]);
    unsigned now = ComputeCrc(plan.mod[MOD_ENGINE]);
    if (now != plan.crcStored)
    {
        sprintf_s(msg, "SendTable CRC recomputed as 0x%08X but the engine holds 0x%08X", now, plan.crcStored);
        return Refuse(quiet, msg);
    }
    return true;
}

// Writes one direction for every item not yet in that state: on = stock -> patched, off = patched -> stock.
// The SendTable CRC is recomputed with the engine's function whenever a SendProp changed. Returns the number of
// failed writes.
int Apply(const Plan& plan, bool on)
{
    int failed = 0;
    const SiteState from = on ? SITE_STOCK : SITE_PATCHED;
    for (const ProbeSite& s : kSites)
    {
        int i = ModIndex(s.module);
        if (i < 0 || !plan.mod[i] || ClassifySite(s, plan.mod[i], plan.delta[i]) != from)
            continue;
        unsigned char bytes[32];
        Expected(s, on ? s.patched : s.stock, plan.delta[i], bytes);
        if (!WriteBytes(reinterpret_cast<unsigned char*>(plan.mod[i]) + s.rva, bytes, s.len))
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
        PropValues nv = on ? LargeProp(kProps[k]) : StockProp(kProps[k]);
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
        unsigned crc = ComputeCrc(plan.mod[MOD_ENGINE]);
        if (WriteBytes(CrcGlobal(plan.mod[MOD_ENGINE]), &crc, 4))
            Out("[bigmaps] SendTable CRC 0x%08X -> 0x%08X\n", plan.crcStored, crc);
        else
        {
            Out("[bigmaps] write failed: SendTable CRC (error %lu)\n", BM_LASTERR());
            ++failed;
        }
    }
    return failed;
}

#ifdef _WIN32
// r_mapextents (client.dll; site P4-12 changes only its default string). The ConVar is registered at startup, so
// its current value was copied from the stock default before any module loaded; keep the live value in step with
// the patch state. Every pointer is checked before the in-place write ("16384" and "32768" have the same length).
// A value the user set by hand is left alone.
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
    float f = on ? kLargeExtent : kStockExtent;
    int n = on ? 32768 : 16384;
    memcpy(str, want, 6);
    memcpy(obj + v.offFloat, &f, 4);
    memcpy(obj + v.offInt, &n, 4);
    Out("[bigmaps] r_mapextents: %s -> %s\n", other, want);
}
#else
void SyncMapExtents(bool) {}  // client-only ConVar; Linux builds are dedicated-server only
#endif

// Verifies, writes whatever is not yet in the wanted state (large = patched), then re-verifies.
void SwitchState(bool large)
{
    Plan plan;
    if (!Verify(plan, false))
        return;
    bool pending = large ? !plan.AllPatched() : !plan.AllStock();
    if (!pending)
    {
        Out("[bigmaps] already %s: nothing to write\n", large ? "patched" : "stock");
        SyncMapExtents(large);
        return;
    }
    int sites = large ? plan.sitesStock : plan.sitesPatched;
    int props = large ? plan.propsStock : plan.propsPatched;
    int failed = Apply(plan, large);
    Plan after;
    bool ok = Verify(after, true) && (large ? after.AllPatched() : after.AllStock());
    Out("[bigmaps] %s: wrote %d sites and %d sendprops, %d write failures; re-check %s\n",
        ok && !failed ? (large ? "APPLIED" : "UNLOADED") : "FAILED", sites, props, failed,
        ok ? (large ? "all patched" : "all stock") : "NOT complete");
    if (ok && !failed)
        SyncMapExtents(large);
}

const char* SizeVerdict(MapSize size)
{
    return size == MAP_LARGE ? "large, enabling" : size == MAP_NORMAL ? "normal, disabling"
                                                                      : "unknown, keeping the current state";
}

}  // namespace
