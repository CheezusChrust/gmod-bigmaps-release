// Map size check: reads the pending map's world bounds through the engine's own file system, so the same file the
// engine is about to load is checked (loose, downloaded, addon or workshop).
#pragma once

#include "platform.h"

namespace bm
{

// Our own declaration of the first five slots of the engine's "VBaseFileSystem011" interface (the
// IBaseFileSystem part of CFileSystem_Stdio). Order verified in this build (MODULE.md): RTTI gives the vtable,
// the slot targets were disassembled (Read/Open forward to ReadEx/OpenEx; Close/Write/Tell/Size report their own
// "FS:  Tried to ... NULL file handle" errors). The provider's hash, vtable and slot RVAs are all checked before
// the first call (CheckProvider).
class IFsBase
{
public:
    virtual int Read(void* out, int size, void* file) = 0;
    virtual int Write(const void* in, int size, void* file) = 0;
    virtual void* Open(const char* name, const char* options, const char* pathId) = 0;
    virtual void Close(void* file) = 0;
    virtual void Seek(void* file, long long pos, int origin) = 0;  // 64-bit position (x86 callee pops 16 bytes); origin 0 = start
};

// The file system lives in dedicated.dll / dedicated_srv.so on the dedicated server and in filesystem_stdio.dll in
// the game; same class either way. The dedicated-server modules do not export it through CreateInterface; their
// own factory returns the pointer from a global (pointerRva).
inline IFsBase* CheckProvider(const FsProvider& p, HMODULE m, char* why, size_t cap)
{
    char hex[65] = "";
    if (!ModuleSha256(m, hex) || _stricmp(hex, p.sha))
    {
        sprintf_s(why, cap, "%s is not the known build (sha256 %s)", p.module, hex[0] ? hex : "unreadable");
        return nullptr;
    }
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
    {
        sprintf_s(why, cap, "%s: VBaseFileSystem011 not available", p.module);
        return nullptr;
    }
    uintptr_t vt = *reinterpret_cast<uintptr_t*>(fs);
    if (vt - b != p.vtableRva || !Readable(reinterpret_cast<void*>(vt), 5 * sizeof(void*)))
    {
        sprintf_s(why, cap, "%s: unexpected file system vtable (rva 0x%llX)", p.module, (unsigned long long)(vt - b));
        return nullptr;
    }
    for (int i = 0; i < 5; ++i)
        if (reinterpret_cast<uintptr_t*>(vt)[i] - b != p.slotRva[i])
        {
            sprintf_s(why, cap, "%s: file system slot %d is not the expected function", p.module, i);
            return nullptr;
        }
    return static_cast<IFsBase*>(fs);
}

// Uses the first provider module that is loaded; on failure `why` says why.
inline IFsBase* GetFileSystem(char* why, size_t cap)
{
    sprintf_s(why, cap, "no file system module loaded");
    for (const FsProvider& p : kFsProviders)
        if (HMODULE m = GetModuleHandleA(p.module))
            return CheckProvider(p, m, why, cap);
    return nullptr;
}

enum MapSize { MAP_UNKNOWN, MAP_NORMAL, MAP_LARGE };

// Reads only the BSP header and the world model (lump 14 = models, model 0: mins[3], maxs[3]) of maps/<name>.bsp
// on the GAME path. Large = any bound beyond the stock limit. Nonsense bounds (not finite or beyond 1e6) give
// MAP_UNKNOWN, as does any read error; `info` describes the result.
inline MapSize CheckMap(IFsBase* fs, const char* name, char* info, size_t cap)
{
    char path[160];
    sprintf_s(path, "maps/%s.bsp", name);
    void* f = fs->Open(path, "rb", "GAME");
    if (!f)
    {
        sprintf_s(info, cap, "%s not found", path);
        return MAP_UNKNOWN;
    }
    MapSize result = MAP_UNKNOWN;
    int header[2 + 64 * 4];  // ident, version, then 64 lumps of {fileofs, filelen, version, uncompressed size}
    if (fs->Read(header, sizeof(header), f) != sizeof(header) || memcmp(header, "VBSP", 4))
        sprintf_s(info, cap, "%s: not a BSP", path);
    else
    {
        const int* lump = header + 2 + 14 * 4;
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
                if (!(b[i] > -1e6f && b[i] < 1e6f))  // also rejects NaN
                    sane = false;
                if ((i < 3 && b[i] < -kStockExtent) || (i >= 3 && b[i] > kStockExtent))
                    large = true;
            }
            sprintf_s(info, cap, "bounds (%.0f %.0f %.0f) .. (%.0f %.0f %.0f)", b[0], b[1], b[2], b[3], b[4], b[5]);
            result = !sane ? MAP_UNKNOWN : large ? MAP_LARGE : MAP_NORMAL;
        }
    }
    fs->Close(f);
    return result;
}

// "maps/Foo.bsp" or "maps\Foo.bsp" or "Foo" -> "foo".
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
