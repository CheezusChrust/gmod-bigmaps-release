// Engine file system access and the BSP size check, shared by the bigmaps modules (Windows x86/x64, Linux x86).
#pragma once

#include "bm_common.h"

namespace bm
{

// ---- Map size check through the engine's file system ----
// The engine file system's "VBaseFileSystem011" interface (the IBaseFileSystem part of CFileSystem_Stdio).
// Our own declaration of the first five slots; order verified in this build (MODULE.md): RTTI gives the
// vtable, the slot targets were disassembled (Read/Open forward to ReadEx/OpenEx, Close/Write/Tell/Size
// report their own "FS:  Tried to ... NULL file handle" errors). The DLL hash, the vtable RVA and the slot
// RVAs are all checked before the first call.
class IFsBase
{
public:
    virtual int Read(void* out, int size, void* file) = 0;
    virtual int Write(const void* in, int size, void* file) = 0;
    virtual void* Open(const char* name, const char* options, const char* pathId) = 0;
    virtual void Close(void* file) = 0;
    virtual void Seek(void* file, long long pos, int origin) = 0;  // 64-bit position (x86 callee pops 16 bytes); origin 0 = start
};

// The file system lives in dedicated.dll on srcds and in filesystem_stdio.dll in the game; same class.
// dedicated.dll does not export the file system through CreateInterface; its own factory returns the
// IBaseFileSystem pointer from a global (pointerRva). filesystem_stdio.dll exports it (pointerRva 0).
// Linux srcds: dedicated_srv.so's FileSystemFactory returns g_pBaseFileSystem (= &g_FileSystem_Stdio + 4, the
// IBaseFileSystem subobject); vtable = CFileSystem_Stdio's secondary vtable, slots = the _ZThn4_ thunks of
// CBaseFileSystem::Read/Write/Open/Close/Seek(void*, long long, FileSystemSeek_t) (LINUX_PORT.md).
struct FsProvider { const char* module; const char* sha; unsigned pointerRva; unsigned vtableRva; unsigned slotRva[5]; };
#if defined(__linux__)
const FsProvider kFsProviders[] = {
    {"dedicated_srv.so", "FA4B05805D2ABFF723867C73C2EE748134393E1CD0595B794F5CE9FB1759F28F", 0x00156438, 0x0011ECD4,
     {0x00045C00, 0x0004B560, 0x00045BB0, 0x00049880, 0x0004C4D0}},
};
#elif defined(_WIN64)
const FsProvider kFsProviders[] = {
    {"dedicated.dll", "571DAF9858DE4BC2088686EB4F5CBC585032EAA4943C4971FAB3DA5BC10757C6", 0x0017E530, 0x00129770,
     {0x00020800, 0x00024870, 0x0001EF70, 0x00013ED0, 0x00022D90}},
    {"filesystem_stdio.dll", "BD03C74FEA015603616EBF3AD8CA0A4A9435C2D50459622F5CFA6EECCF41F7FB", 0, 0x000B2B10,
     {0x00020970, 0x00024B50, 0x0001EF80, 0x00013CC0, 0x00022F00}},
};
#else
const FsProvider kFsProviders[] = {
    {"dedicated.dll", "2882D259DB67B195D0FCF07845D54B1BE47A872A11E5D885A018E0B38A076E9B", 0x0011F330, 0x000F0394,
     {0x00019B30, 0x0001D1E0, 0x00018940, 0x000109C0, 0x0001BA00}},
    {"filesystem_stdio.dll", "4F7EEDB8FE096B6DADDB06162FB243032EDEC72FE9AAE4492C2685787AAED925", 0, 0x00093B24,
     {0x000192F0, 0x0001CB00, 0x00017F70, 0x00010000, 0x0001B1C0}},
};
#endif

inline IFsBase* CheckProvider(const FsProvider& p, HMODULE m, char* why, size_t cap)
{
    char hex[65] = "";
    if (!ModuleSha256(m, hex) || _stricmp(hex, p.sha))
        return sprintf_s(why, cap, "%s is not the known build (sha256 %s)", p.module, hex[0] ? hex : "unreadable"),
               nullptr;
    uintptr_t b = reinterpret_cast<uintptr_t>(m);
    void* fs = nullptr;
    if (p.pointerRva)
    {
        void** slot = reinterpret_cast<void**>(b + p.pointerRva);
        fs = Readable(slot, sizeof(void*)) ? *slot : nullptr;
    }
#ifdef _WIN32
    else
    {
        typedef void*(__cdecl * CreateInterfaceFn)(const char* name, int* rc);
        auto create = reinterpret_cast<CreateInterfaceFn>(GetProcAddress(m, "CreateInterface"));
        fs = create ? create("VBaseFileSystem011", nullptr) : nullptr;
    }
#endif
    if (!fs || !Readable(fs, sizeof(void*)))
        return sprintf_s(why, cap, "%s: VBaseFileSystem011 not available", p.module), nullptr;
    uintptr_t vt = *reinterpret_cast<uintptr_t*>(fs);
    if (vt - b != p.vtableRva || !Readable(reinterpret_cast<void*>(vt), 5 * sizeof(void*)))
        return sprintf_s(why, cap, "%s: unexpected file system vtable (rva 0x%llX)", p.module,
                         (unsigned long long)(vt - b)),
               nullptr;
    for (int i = 0; i < 5; ++i)
        if (reinterpret_cast<uintptr_t*>(vt)[i] - b != p.slotRva[i])
            return sprintf_s(why, cap, "%s: file system slot %d is not the expected function", p.module, i), nullptr;
    return static_cast<IFsBase*>(fs);
}

inline IFsBase* GetFileSystem(char* why, size_t cap)
{
    sprintf_s(why, cap, "no file system module loaded");
    for (const FsProvider& p : kFsProviders)
        if (HMODULE m = GetModuleHandleA(p.module))
            return CheckProvider(p, m, why, cap);
    return nullptr;
}

enum MapSize { MAP_UNKNOWN, MAP_NORMAL, MAP_LARGE };

// Reads only the BSP header and the world model (lump 14, model 0: mins[3], maxs[3]) of maps/<name>.bsp on
// the GAME path, i.e. the file the engine is about to load (loose, download, addon or workshop). Large =
// any bound beyond +-16384, the limit the stock engine's extents check enforces.
inline MapSize CheckMap(IFsBase* fs, const char* name, char* info, size_t cap)
{
    char path[160];
    sprintf_s(path, "maps/%s.bsp", name);
    void* f = fs->Open(path, "rb", "GAME");
    if (!f)
        return sprintf_s(info, cap, "%s not found", path), MAP_UNKNOWN;
    MapSize result = MAP_UNKNOWN;
    int header[2 + 64 * 4];
    if (fs->Read(header, sizeof(header), f) != sizeof(header) || memcmp(header, "VBSP", 4))
        sprintf_s(info, cap, "%s: not a BSP", path);
    else
    {
        const int* lump = header + 2 + 14 * 4;  // fileofs, filelen, version, uncompressed size
        float b[6];
        if (lump[1] < 24 || lump[3] != 0)
            sprintf_s(info, cap, "%s: world model lump %s", path, lump[3] ? "compressed" : "missing");
        else if (fs->Seek(f, static_cast<long long>(lump[0]), 0), fs->Read(b, sizeof(b), f) != sizeof(b))
            sprintf_s(info, cap, "%s: world model unreadable", path);
        else
        {
            bool large = false, sane = true;
            for (int i = 0; i < 6; ++i)
            {
                if (!(b[i] > -1e6f && b[i] < 1e6f))
                    sane = false;
                if ((i < 3 && b[i] < -16384.0f) || (i >= 3 && b[i] > 16384.0f))
                    large = true;
            }
            sprintf_s(info, cap, "bounds (%.0f %.0f %.0f) .. (%.0f %.0f %.0f)", b[0], b[1], b[2], b[3], b[4], b[5]);
            result = !sane ? MAP_UNKNOWN : large ? MAP_LARGE : MAP_NORMAL;
        }
    }
    fs->Close(f);
    return result;
}


inline void NormalizeMapName(const char* in, char* out, size_t cap)
{
    if (!_strnicmp(in, "maps/", 5) || !_strnicmp(in, "maps\\", 5))
        in += 5;
    size_t n = 0;
    for (; in[n] && n + 1 < cap; ++n)
        out[n] = static_cast<char>(tolower(static_cast<unsigned char>(in[n])));
    out[n] = 0;
    if (n > 4 && !strcmp(out + n - 4, ".bsp"))
        out[n - 4] = 0;
}

}  // namespace bm
