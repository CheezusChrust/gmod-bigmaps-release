#define BM_BRANCH_SYNTAX_CHECK 1
#include "../common/patch_sites.h"
#include <cstdio>

HMODULE Image()
{
    const DWORD size = 0x01000000;
    unsigned char* image = static_cast<unsigned char*>(VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE,
                                                                    PAGE_READWRITE));
    if (!image)
        return nullptr;
    IMAGE_DOS_HEADER* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(image);
    dos->e_lfanew = 0x100;
    IMAGE_NT_HEADERS* nt = reinterpret_cast<IMAGE_NT_HEADERS*>(image + dos->e_lfanew);
    nt->OptionalHeader.SizeOfImage = size;
    return reinterpret_cast<HMODULE>(image);
}

int main()
{
    HMODULE engine = Image(), client = Image(), server = Image();
    if (!engine || !client || !server)
        return 1;
    for (const bm::ProbeSite& site : bm::kSites)
    {
        HMODULE image = !strcmp(site.module, "engine.dll") ? engine :
                        !strcmp(site.module, "client.dll") ? client :
                        !strcmp(site.module, "server.dll") ? server : nullptr;
        if (!image || site.relocMask || site.rva + site.len > bm::ImageSize(image) ||
            !memcmp(site.stock, site.patched, site.len))
            return 2;
        unsigned char* location = reinterpret_cast<unsigned char*>(image) + site.rva;
        memcpy(location, site.stock, site.len);
        if (bm::ClassifySite(site, image, 0) != bm::SITE_STOCK)
            return 3;
        if (!bm::WriteBytes(location, site.patched, site.len) ||
            bm::ClassifySite(site, image, 0) != bm::SITE_PATCHED)
            return 4;
        location[0] ^= 0xFF;
        if (bm::ClassifySite(site, image, 0) != bm::SITE_OTHER)
            return 5;
        if (!bm::WriteBytes(location, site.stock, site.len) ||
            bm::ClassifySite(site, image, 0) != bm::SITE_STOCK)
            return 6;
    }
    VirtualFree(engine, 0, MEM_RELEASE);
    VirtualFree(client, 0, MEM_RELEASE);
    VirtualFree(server, 0, MEM_RELEASE);
    std::printf("%zu candidate sites: stock, patched, tampered and restored states OK\n",
                sizeof(bm::kSites) / sizeof(bm::kSites[0]));
    return 0;
}