// Shared helpers for the bigger-maps modules (Windows x86/x64; Linux x86 dedicated server). Header-only.
// Linux (srcds_linux, 32-bit): HMODULE is the module's load base (link base 0), found by file name with
// dl_iterate_phdr; memory state comes from /proc/self/maps; section ranges come from kImages (generated,
// checked against the file hash). GCC's i386 default calling convention is cdecl.
#pragma once

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#define BM_EXPORT extern "C" __declspec(dllexport)
#define BM_LASTERR() static_cast<unsigned long>(GetLastError())
#else
#include <ctype.h>
#include <dlfcn.h>
#include <errno.h>
#include <link.h>
#include <stdarg.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/mman.h>
#include <unistd.h>
#define BM_EXPORT extern "C" __attribute__((visibility("default")))
#define BM_LASTERR() static_cast<unsigned long>(errno)
#define __cdecl
struct BmModule;
typedef BmModule* HMODULE;  // load base
typedef unsigned long DWORD;
#define _stricmp strcasecmp
#define _strnicmp strncasecmp
template <size_t N> __attribute__((format(printf, 2, 3))) inline int sprintf_s(char (&b)[N], const char* f, ...)
{
    va_list ap;
    va_start(ap, f);
    int r = vsnprintf(b, N, f, ap);
    va_end(ap);
    return r;
}
__attribute__((format(printf, 3, 4))) inline int sprintf_s(char* b, size_t n, const char* f, ...)
{
    va_list ap;
    va_start(ap, f);
    int r = vsnprintf(b, n, f, ap);
    va_end(ap);
    return r;
}
#endif

#include "sites.h"

namespace bm {

#ifdef _WIN32
const char* const kTier0 = "tier0.dll";
const char* const kDedicated = "dedicated.dll";
#else
const char* const kTier0 = "libtier0_srv.so";
const char* const kDedicated = "dedicated_srv.so";

// ---- Linux module lookup and memory map ----
struct FindCtx { const char* name; uintptr_t base; };

inline int FindCb(struct dl_phdr_info* info, size_t, void* data)
{
    FindCtx* c = static_cast<FindCtx*>(data);
    const char* path = info->dlpi_name;
    if (!path || !*path)
        return 0;
    const char* slash = strrchr(path, '/');
    if (strcmp(slash ? slash + 1 : path, c->name))
        return 0;
    c->base = static_cast<uintptr_t>(info->dlpi_addr);
    return 1;
}

inline HMODULE GetModuleHandleA(const char* name)
{
    FindCtx c = {name, 0};
    dl_iterate_phdr(FindCb, &c);
    return reinterpret_cast<HMODULE>(c.base);
}

struct MapRegion { uintptr_t lo, hi; int prot; };
struct MapSnapshot { MapRegion r[8192]; int n; };

inline MapSnapshot& Maps()
{
    static MapSnapshot s;
    return s;
}

inline void RefreshMaps()
{
    MapSnapshot& s = Maps();
    s.n = 0;
    FILE* f = fopen("/proc/self/maps", "r");
    if (!f)
        return;
    char line[512];
    while (s.n < 8192 && fgets(line, sizeof(line), f))
    {
        unsigned long lo, hi;
        char perm[8];
        if (sscanf(line, "%lx-%lx %7s", &lo, &hi, perm) != 3)
            continue;
        s.r[s.n++] = {lo, hi, (perm[0] == 'r' ? PROT_READ : 0) | (perm[1] == 'w' ? PROT_WRITE : 0) |
                                  (perm[2] == 'x' ? PROT_EXEC : 0)};
    }
    fclose(f);
}

// Protection of the page holding p, or -1 if unmapped. Refreshes the snapshot once on a miss.
inline int PageProt(uintptr_t p, bool refreshed = false)
{
    const MapSnapshot& s = Maps();
    for (int i = 0; i < s.n; ++i)
        if (p >= s.r[i].lo && p < s.r[i].hi)
            return s.r[i].prot;
    if (refreshed)
        return -1;
    RefreshMaps();
    return PageProt(p, true);
}

// Absolute path of the file mapped at `base` (first mapping of a module starts at its load base).
inline bool ModulePath(HMODULE m, char* out, size_t cap)
{
    FILE* f = fopen("/proc/self/maps", "r");
    if (!f)
        return false;
    char line[1024];
    bool ok = false;
    while (!ok && fgets(line, sizeof(line), f))
    {
        unsigned long lo;
        int pos = 0;
        if (sscanf(line, "%lx-%*x %*s %*s %*s %*s %n", &lo, &pos) < 1 || !pos || lo != reinterpret_cast<uintptr_t>(m))
            continue;
        char* p = line + pos;
        p[strcspn(p, "\n")] = 0;
        if (*p == '/')
            ok = snprintf(out, cap, "%s", p) < static_cast<int>(cap);
    }
    fclose(f);
    return ok;
}

inline const ProbeImage* ImageOf(HMODULE m)
{
    for (const ProbeImage& i : kImages)
        if (GetModuleHandleA(i.module) == m)
            return &i;
    return nullptr;
}
#endif

typedef void (*MsgFn)(const char* fmt, ...);
inline MsgFn& MsgPtr()
{
    static MsgFn fn = nullptr;
    return fn;
}

inline void InitConsole()
{
#ifdef _WIN32
    if (HMODULE tier0 = GetModuleHandleA(kTier0))
        MsgPtr() = reinterpret_cast<MsgFn>(GetProcAddress(tier0, "Msg"));
#else
    if (void* h = dlopen(kTier0, RTLD_NOW | RTLD_NOLOAD))
    {
        MsgPtr() = reinterpret_cast<MsgFn>(dlsym(h, "Msg"));
        dlclose(h);
    }
#endif
}

inline void Out(const char* fmt, ...)
{
    char buf[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    if (MsgPtr())
        MsgPtr()("%s", buf);
    else
#ifdef _WIN32
        OutputDebugStringA(buf);
#else
        fputs(buf, stderr);
#endif
}

#ifdef _WIN32
inline bool Readable(const void* p, size_t n)
{
    const char* a = static_cast<const char*>(p);
    const char* end = a + n;
    while (a < end)
    {
        MEMORY_BASIC_INFORMATION mbi;
        if (!VirtualQuery(a, &mbi, sizeof(mbi)) || mbi.State != MEM_COMMIT ||
            (mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD)) || mbi.Protect == 0)
            return false;
        a = static_cast<const char*>(mbi.BaseAddress) + mbi.RegionSize;
    }
    return true;
}

inline DWORD Protection(const void* p)
{
    MEMORY_BASIC_INFORMATION mbi;
    return VirtualQuery(p, &mbi, sizeof(mbi)) ? mbi.Protect : 0;
}

inline const char* ProtName(DWORD p)
{
    switch (p & 0xFF)
    {
    case PAGE_READONLY: return "R";
    case PAGE_READWRITE: return "RW";
    case PAGE_WRITECOPY: return "WC";
    case PAGE_EXECUTE: return "X";
    case PAGE_EXECUTE_READ: return "RX";
    case PAGE_EXECUTE_READWRITE: return "RWX";
    case PAGE_EXECUTE_WRITECOPY: return "XWC";
    default: return "?";
    }
}
#else
inline bool Readable(const void* p, size_t n)
{
    uintptr_t a = reinterpret_cast<uintptr_t>(p), end = a + n;
    if (end < a)
        return false;
    while (a < end)
    {
        int prot = PageProt(a);
        if (prot < 0 || !(prot & PROT_READ))
            return false;
        const MapSnapshot& s = Maps();
        uintptr_t next = 0;
        for (int i = 0; i < s.n && !next; ++i)
            if (a >= s.r[i].lo && a < s.r[i].hi)
                next = s.r[i].hi;
        if (!next)
            return false;
        a = next;
    }
    return true;
}
#endif

#ifdef _WIN32
inline bool FileSha256(const wchar_t* path, char hex[65])
{
    hex[0] = 0;
    HANDLE f = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE, nullptr,
                           OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
    if (f == INVALID_HANDLE_VALUE)
        return false;
    BCRYPT_ALG_HANDLE alg = nullptr;
    BCRYPT_HASH_HANDLE h = nullptr;
    bool ok = BCryptOpenAlgorithmProvider(&alg, BCRYPT_SHA256_ALGORITHM, nullptr, 0) == 0 &&
              BCryptCreateHash(alg, &h, nullptr, 0, nullptr, 0, 0) == 0;
    static unsigned char buf[1 << 16];
    DWORD got = 0;
    while (ok && ReadFile(f, buf, sizeof(buf), &got, nullptr) && got)
        ok = BCryptHashData(h, buf, got, 0) == 0;
    unsigned char digest[32];
    ok = ok && BCryptFinishHash(h, digest, sizeof(digest), 0) == 0;
    if (h) BCryptDestroyHash(h);
    if (alg) BCryptCloseAlgorithmProvider(alg, 0);
    CloseHandle(f);
    if (ok)
        for (int i = 0; i < 32; ++i)
            sprintf_s(hex + i * 2, 3, "%02X", digest[i]);
    return ok;
}

inline const IMAGE_NT_HEADERS* Nt(HMODULE m)
{
    auto dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(m);
    return reinterpret_cast<const IMAGE_NT_HEADERS*>(reinterpret_cast<const char*>(m) + dos->e_lfanew);
}

inline bool ModuleSha256(HMODULE m, char hex[65])
{
    wchar_t path[MAX_PATH * 2];
    DWORD n = GetModuleFileNameW(m, path, ARRAYSIZE(path));
    hex[0] = 0;
    return n && n < ARRAYSIZE(path) && FileSha256(path, hex);
}

inline uintptr_t ImageSize(HMODULE m)
{
    return Nt(m)->OptionalHeader.SizeOfImage;
}
#else
// FIPS 180-4 SHA-256 (file identity only).
struct Sha256
{
    uint32_t h[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
    unsigned char blk[64];
    size_t used = 0;
    uint64_t total = 0;

    static uint32_t Rotr(uint32_t x, int n) { return (x >> n) | (x << (32 - n)); }

    void Block(const unsigned char* p)
    {
        static const uint32_t k[64] = {
            0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
            0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
            0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
            0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
            0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
            0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
            0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
            0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};
        uint32_t w[64];
        for (int i = 0; i < 16; ++i)
            w[i] = uint32_t(p[4 * i]) << 24 | uint32_t(p[4 * i + 1]) << 16 | uint32_t(p[4 * i + 2]) << 8 | p[4 * i + 3];
        for (int i = 16; i < 64; ++i)
        {
            uint32_t s0 = Rotr(w[i - 15], 7) ^ Rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
            uint32_t s1 = Rotr(w[i - 2], 17) ^ Rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
        }
        uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4], f = h[5], g = h[6], hh = h[7];
        for (int i = 0; i < 64; ++i)
        {
            uint32_t t1 = hh + (Rotr(e, 6) ^ Rotr(e, 11) ^ Rotr(e, 25)) + ((e & f) ^ (~e & g)) + k[i] + w[i];
            uint32_t t2 = (Rotr(a, 2) ^ Rotr(a, 13) ^ Rotr(a, 22)) + ((a & b) ^ (a & c) ^ (b & c));
            hh = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
        }
        h[0] += a; h[1] += b; h[2] += c; h[3] += d; h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
    }

    void Update(const unsigned char* p, size_t n)
    {
        total += n;
        while (n)
        {
            size_t take = 64 - used < n ? 64 - used : n;
            memcpy(blk + used, p, take);
            used += take; p += take; n -= take;
            if (used == 64)
            {
                Block(blk);
                used = 0;
            }
        }
    }

    void Final(unsigned char out[32])
    {
        uint64_t bits = total * 8;
        unsigned char pad = 0x80, zero = 0;
        Update(&pad, 1);
        while (used != 56)
            Update(&zero, 1);
        unsigned char len[8];
        for (int i = 0; i < 8; ++i)
            len[i] = static_cast<unsigned char>(bits >> (56 - 8 * i));
        Update(len, 8);
        for (int i = 0; i < 8; ++i)
            for (int j = 0; j < 4; ++j)
                out[4 * i + j] = static_cast<unsigned char>(h[i] >> (24 - 8 * j));
    }
};

inline bool FileSha256(const char* path, char hex[65])
{
    hex[0] = 0;
    FILE* f = fopen(path, "rb");
    if (!f)
        return false;
    Sha256 sha;
    static unsigned char buf[1 << 16];
    size_t got;
    while ((got = fread(buf, 1, sizeof(buf), f)) > 0)
        sha.Update(buf, got);
    bool ok = !ferror(f);
    fclose(f);
    unsigned char digest[32];
    sha.Final(digest);
    if (ok)
        for (int i = 0; i < 32; ++i)
            snprintf(hex + i * 2, 3, "%02X", digest[i]);
    return ok;
}

inline bool ModuleSha256(HMODULE m, char hex[65])
{
    char path[1024];
    hex[0] = 0;
    return ModulePath(m, path, sizeof(path)) && FileSha256(path, hex);
}

inline uintptr_t ImageSize(HMODULE m)
{
    const ProbeImage* i = ImageOf(m);
    return i ? i->size : 0;
}
#endif

inline const ProbeFile* FileInfo(const char* name)
{
    for (const ProbeFile& f : kFiles)
        if (!strcmp(f.module, name))
            return &f;
    return nullptr;
}

enum FileId { BMF_UNREADABLE, BMF_STOCK, BMF_PATCHED, BMF_UNKNOWN };

inline FileId IdentifyFile(const char* name, HMODULE m, char hex[65])
{
    if (!ModuleSha256(m, hex))
        return BMF_UNREADABLE;
    const ProbeFile* fi = FileInfo(name);
    if (!fi)
        return BMF_UNKNOWN;
    return !strcmp(hex, fi->stockSha) ? BMF_STOCK : !strcmp(hex, fi->patchedSha) ? BMF_PATCHED : BMF_UNKNOWN;
}

inline const char* FileIdName(FileId id)
{
    switch (id)
    {
    case BMF_STOCK: return "stock";
    case BMF_PATCHED: return "previously patched";
    case BMF_UNKNOWN: return "UNKNOWN BUILD";
    default: return "UNREADABLE";
    }
}

inline uintptr_t LoadDelta(const char* name, HMODULE m)
{
    const ProbeFile* fi = FileInfo(name);
    return reinterpret_cast<uintptr_t>(m) - static_cast<uintptr_t>(fi ? fi->preferredBase : 0);
}

// Expected bytes of a site at this load base: relocated values shift by (actual - preferred).
inline void Expected(const ProbeSite& s, const unsigned char* src, uintptr_t delta, unsigned char* out)
{
    memcpy(out, src, s.len);
    for (unsigned i = 0; i < s.len && i < 32; ++i)
    {
        if (!(s.relocMask & (1u << i)))
            continue;
        uintptr_t v;
        memcpy(&v, out + i, sizeof(v));
        v += delta;
        memcpy(out + i, &v, sizeof(v));
    }
}

enum SiteState { SITE_UNREADABLE, SITE_STOCK, SITE_PATCHED, SITE_OTHER };

inline SiteState ClassifySite(const ProbeSite& s, HMODULE m, uintptr_t delta)
{
    const unsigned char* at = reinterpret_cast<const unsigned char*>(m) + s.rva;
    if (s.rva + s.len > ImageSize(m) || !Readable(at, s.len))
        return SITE_UNREADABLE;
    unsigned char es[32], ep[32];
    Expected(s, s.stock, delta, es);
    Expected(s, s.patched, delta, ep);
    if (!memcmp(at, ep, s.len))
        return SITE_PATCHED;
    if (!memcmp(at, es, s.len))
        return SITE_STOCK;
    return SITE_OTHER;
}

// ---- SendTable / SendProp access (server.dll) ----
// Field offsets re-derived from the binaries (PATCH_LOG x86/x64 SendProp layout); the name offset
// follows the public SDK member order and is validated by the stride check in FindSendTable.
// Linux server_srv.so (GCC) has the same 32-bit layout (SendPropFloat, SendTable::Construct; LINUX_PORT.md).
#if defined(_WIN64)
const size_t kPropBits = 0x14, kPropLow = 0x18, kPropHigh = 0x1C, kPropVarName = 0x48, kPropHLMul = 0x50,
             kPropFlags = 0x54;
const size_t kTblCount = 0x08, kTblName = 0x10;
#else
const size_t kPropBits = 0x0C, kPropLow = 0x10, kPropHigh = 0x14, kPropVarName = 0x30, kPropHLMul = 0x34,
             kPropFlags = 0x38;
const size_t kTblCount = 0x04, kTblName = 0x08;
#endif

struct Range { uintptr_t lo, hi; };

inline Range ImageRange(HMODULE m)
{
    return {reinterpret_cast<uintptr_t>(m), reinterpret_cast<uintptr_t>(m) + ImageSize(m)};
}

// Calls fn(start, size) for each searched region: read-only data for strings (PE: non-executable, non-writable
// sections; ELF: .rodata, which shares the executable segment), or writable data for SendTables.
template <typename Fn> inline void ForEachRegion(HMODULE m, bool writable, Fn fn)
{
#ifdef _WIN32
    const IMAGE_NT_HEADERS* nt = Nt(m);
    const IMAGE_SECTION_HEADER* sec = IMAGE_FIRST_SECTION(nt);
    for (unsigned i = 0; i < nt->FileHeader.NumberOfSections; ++i)
    {
        DWORD ch = sec[i].Characteristics;
        if (writable ? !(ch & IMAGE_SCN_MEM_WRITE) : (ch & (IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_WRITE)) != 0)
            continue;
        fn(reinterpret_cast<uintptr_t>(m) + sec[i].VirtualAddress, static_cast<size_t>(sec[i].Misc.VirtualSize));
    }
#else
    if (const ProbeImage* i = ImageOf(m))
        fn(reinterpret_cast<uintptr_t>(m) + (writable ? i->rwRva : i->roRva), static_cast<size_t>(writable ? i->rwSize : i->roSize));
#endif
}

inline bool InImageString(Range img, uintptr_t p, const char** out)
{
    if (p < img.lo || p >= img.hi)
        return false;
    const char* s = reinterpret_cast<const char*>(p);
    for (int i = 0; i < 128; ++i)
    {
        if (!Readable(s + i, 1))
            return false;
        if (!s[i])
        {
            if (!i)
                return false;
            *out = s;
            return true;
        }
        if (s[i] < 0x20 || s[i] > 0x7E)
            return false;
    }
    return false;
}

inline uintptr_t FindString(HMODULE m, const char* str)
{
    size_t len = strlen(str) + 1;
    uintptr_t found = 0;
    ForEachRegion(m, false, [&](uintptr_t start, size_t n) {
        const char* a = reinterpret_cast<const char*>(start);
        if (found || !Readable(a, n))
            return;
        for (size_t k = 1; k + len <= n; ++k)
            if (a[k - 1] == 0 && !memcmp(a + k, str, len))
            {
                found = reinterpret_cast<uintptr_t>(a + k);
                return;
            }
    });
    return found;
}

struct SendTableRef
{
    uintptr_t table = 0, props = 0;
    int count = 0;
    size_t stride = 0;
    int matches = 0;  // number of valid table objects found with this name
};

// Finds the SendTable object whose name pointer refers to the string `name` in server.dll.
inline SendTableRef FindSendTable(HMODULE m, const char* name)
{
    SendTableRef r;
    Range img = ImageRange(m);
    uintptr_t str = FindString(m, name);
    if (!str)
        return r;
    ForEachRegion(m, true, [&](uintptr_t a, size_t n) {
        n &= ~(sizeof(void*) - 1);
        if (!Readable(reinterpret_cast<void*>(a), n))
            return;
        for (size_t k = kTblName; k + sizeof(void*) <= n; k += sizeof(void*))
        {
            if (*reinterpret_cast<uintptr_t*>(a + k) != str)
                continue;
            uintptr_t t = a + k - kTblName;
            uintptr_t pprops = *reinterpret_cast<uintptr_t*>(t);
            int count = *reinterpret_cast<int*>(t + kTblCount);
            if (count < 1 || count > 1024 || pprops < img.lo || pprops >= img.hi)
                continue;
            size_t stride = 0;
            for (size_t cand = kPropFlags + 4; cand <= 0x200 && !stride; cand += sizeof(void*))
            {
                if (!Readable(reinterpret_cast<void*>(pprops), cand * count))
                    break;
                bool ok = true;
                const char* nm;
                for (int j = 0; j < count && ok; ++j)
                    ok = InImageString(img, *reinterpret_cast<uintptr_t*>(pprops + j * cand + kPropVarName), &nm);
                if (ok)
                    stride = cand;
            }
            if (!stride)
                continue;
            ++r.matches;
            r.table = t;
            r.props = pprops;
            r.count = count;
            r.stride = stride;
        }
    });
    return r;
}

// Address of the prop named `var` in the table, or 0.
inline uintptr_t FindProp(HMODULE m, const SendTableRef& t, const char* var)
{
    Range img = ImageRange(m);
    for (int j = 0; j < t.count; ++j)
    {
        uintptr_t p = t.props + j * t.stride;
        const char* nm;
        if (InImageString(img, *reinterpret_cast<uintptr_t*>(p + kPropVarName), &nm) && !strcmp(nm, var))
            return p;
    }
    return 0;
}

struct PropValues { int bits; float low, high, mul; };

inline PropValues ReadProp(uintptr_t p)
{
    return {*reinterpret_cast<int*>(p + kPropBits), *reinterpret_cast<float*>(p + kPropLow),
            *reinterpret_cast<float*>(p + kPropHigh), *reinterpret_cast<float*>(p + kPropHLMul)};
}

// Range multiplier for a quantized float: (2^bits - 1) / (high - low). Exact in float for the
// ranges used here (power-of-two spans), and equal to the value the game stores (probe results).
inline PropValues RangeValues(int bits, float span)
{
    return {bits, -span, span, static_cast<float>(((1u << bits) - 1) / (2.0 * span))};
}

inline bool SameProp(const PropValues& a, const PropValues& b)
{
    return a.bits == b.bits && a.low == b.low && a.high == b.high && a.mul == b.mul;
}

// ---- Server SendTable CRC (engine.dll) ----
// The engine computes it once in SendTable_Init (before any module loads) and compares it with
// each joining client's. kAnchors verifies the getter, the CRC function and SendTable_Init's
// store; the CRC function is a pure CRC32 over the registered SendTables (no arguments).
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

inline unsigned ComputeCrc(HMODULE eng)
{
    typedef unsigned(__cdecl * CrcFn)();
    return reinterpret_cast<CrcFn>(reinterpret_cast<uintptr_t>(eng) + kCrcFuncRva)();
}

// Engine host-state block (verified by the hs_name / hs_state anchors): +0 current state, +4 next state,
// +0x20 level name (0x100 bytes). States used here: 0 = new game (`map`), 3 = `changelevel`.
struct HostStateView { int current; int next; char level[256]; };

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
