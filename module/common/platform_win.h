// Windows OS layer (x86 and x64; game and dedicated server): memory queries and writes, module file hashing and
// PE section walking.
#pragma once

#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")

#define BM_EXPORT extern "C" __declspec(dllexport)
#define BM_LASTERR() static_cast<unsigned long>(GetLastError())

#include "../targets/targets.h"

namespace bm
{

const char* const kTier0 = "tier0.dll";
const char* const kDedicated = "dedicated.dll";

// True if all of [p, p + n) is committed and accessible; walks across region boundaries.
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

// Hashes the module's file on disk (identifies the build; the in-memory image is relocated and patched).
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

// Calls fn(start, size) for each section searched: read-only data (neither executable nor writable) for strings,
// or writable data for SendTables.
template <typename Fn> inline void ForEachRegion(HMODULE m, bool writable, Fn fn)
{
    const IMAGE_NT_HEADERS* nt = Nt(m);
    const IMAGE_SECTION_HEADER* sec = IMAGE_FIRST_SECTION(nt);
    for (unsigned i = 0; i < nt->FileHeader.NumberOfSections; ++i)
    {
        DWORD ch = sec[i].Characteristics;
        if (writable ? !(ch & IMAGE_SCN_MEM_WRITE) : (ch & (IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_WRITE)) != 0)
            continue;
        fn(reinterpret_cast<uintptr_t>(m) + sec[i].VirtualAddress, static_cast<size_t>(sec[i].Misc.VirtualSize));
    }
}

// Copies n bytes over code or data. Code pages get PAGE_EXECUTE_READWRITE rather than PAGE_READWRITE so other
// threads running there do not fault; the old protection is restored afterwards.
inline bool WriteBytes(void* at, const void* src, size_t n)
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

}  // namespace bm
