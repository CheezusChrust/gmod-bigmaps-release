// SendTable / SendProp access in the server module. Field offsets (kProp*, kTbl*) come from the target data.
#pragma once

#include "platform.h"

namespace bm
{

struct Range { uintptr_t lo, hi; };

inline Range ImageRange(HMODULE m)
{
    return {reinterpret_cast<uintptr_t>(m), reinterpret_cast<uintptr_t>(m) + ImageSize(m)};
}

// True if p points into the image at a short, printable, NUL-terminated string (a plausible name).
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

// Address of the first whole string `str` (preceded by a NUL) in the module's read-only data, or 0.
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

// Finds the SendTable object whose name pointer refers to the string `name`, by scanning writable data for that
// pointer. A candidate counts only if its prop array is in the image and some stride (the SendProp size, not
// known from headers) gives every prop a plausible name; the smallest such stride is used. Callers require
// exactly one match.
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
            uintptr_t pprops = *reinterpret_cast<uintptr_t*>(t);  // the prop array pointer is the first field
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

// The quantization fields of a float SendProp: bit count, range and high-low multiplier.
struct PropValues { int bits; float low, high, mul; };

inline PropValues ReadProp(uintptr_t p)
{
    return {*reinterpret_cast<int*>(p + kPropBits), *reinterpret_cast<float*>(p + kPropLow),
            *reinterpret_cast<float*>(p + kPropHigh), *reinterpret_cast<float*>(p + kPropHLMul)};
}

// Values for a symmetric range -span..+span: multiplier = (2^bits - 1) / (high - low). Exact in float for the
// power-of-two spans used here, and equal to the value the game stores (probe results).
inline PropValues RangeValues(int bits, float span)
{
    return {bits, -span, span, static_cast<float>(((1u << bits) - 1) / (2.0 * span))};
}

inline bool SameProp(const PropValues& a, const PropValues& b)
{
    return a.bits == b.bits && a.low == b.low && a.high == b.high && a.mul == b.mul;
}

}  // namespace bm
