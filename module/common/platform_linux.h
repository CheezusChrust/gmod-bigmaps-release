// Linux x86 OS layer (srcds_linux, 32-bit; dedicated server only). Provides the few Win32 names the shared code
// uses, so that code reads the same on both systems:
//   HMODULE            the module's load base (ELF modules are linked at 0, so RVAs are offsets from it), found
//                      by file name with dl_iterate_phdr;
//   memory state       from /proc/self/maps;
//   image size/ranges  from kImages (target data, valid only for the hashed files).
#pragma once

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
#define __cdecl  // GCC's i386 default calling convention is already cdecl
struct BmModule;
typedef BmModule* HMODULE;
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

#include "../targets/targets.h"
#include "sha256.h"

namespace bm
{

const char* const kTier0 = "libtier0_srv.so";
const char* const kDedicated = "dedicated_srv.so";

// ---- Module lookup ----
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

// ---- Memory map ----
// A cached snapshot of /proc/self/maps; refreshed on a miss and before each write.
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

// Absolute path of the file mapped at the module's base (a module's first mapping starts at its load base).
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

// ---- File identity ----
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

// Calls fn(start, size) for the range searched: .rodata for strings (it shares the executable segment, so it
// cannot be told apart by protection), or the writable data for SendTables.
template <typename Fn> inline void ForEachRegion(HMODULE m, bool writable, Fn fn)
{
    if (const ProbeImage* i = ImageOf(m))
        fn(reinterpret_cast<uintptr_t>(m) + (writable ? i->rwRva : i->roRva), static_cast<size_t>(writable ? i->rwSize : i->roSize));
}

// ---- Writing ----
// Adds write access to each page touched (keeping its read/execute bits, so other threads can still run code
// there), copies, then restores each page's original protection from /proc/self/maps.
inline bool WriteBytes(void* at, const void* src, size_t n)
{
    const uintptr_t page = static_cast<uintptr_t>(sysconf(_SC_PAGESIZE));
    const uintptr_t first = reinterpret_cast<uintptr_t>(at) & ~(page - 1);
    const uintptr_t last = (reinterpret_cast<uintptr_t>(at) + n - 1) & ~(page - 1);
    int prot[2] = {-1, -1};
    if (last - first > page)
        return false;  // sites are at most 32 bytes, so at most two pages
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

}  // namespace bm
