// Module identity (stock build check by file hash) and patch-site classification.
#pragma once

#include "platform.h"

namespace bm
{

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

// Actual load base minus the base the site bytes were recorded at (0 on Linux, where RVAs are base offsets).
inline uintptr_t LoadDelta(const char* name, HMODULE m)
{
    const ProbeFile* fi = FileInfo(name);
    return reinterpret_cast<uintptr_t>(m) - static_cast<uintptr_t>(fi ? fi->preferredBase : 0);
}

// Expected bytes of a site at this load base: each relocated pointer-sized value shifts by the load delta.
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

// Patched is tested first: for anchors and hook prologues stock == patched, and callers expect SITE_PATCHED.
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

}  // namespace bm
