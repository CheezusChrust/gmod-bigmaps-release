#define BM_BRANCH_SYNTAX_CHECK 1
#include "../common/patch_sites.h"
#include "../bigmaps/sound_hook_win.h"
#include <cstdio>

int main()
{
    const size_t imageSize = 0x00400000;
    unsigned char* image = static_cast<unsigned char*>(VirtualAlloc(nullptr, imageSize, MEM_COMMIT | MEM_RESERVE,
                                                                    PAGE_READWRITE));
    if (!image)
        return 1;
    IMAGE_DOS_HEADER* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(image);
    dos->e_lfanew = 0x100;
    IMAGE_NT_HEADERS* nt = reinterpret_cast<IMAGE_NT_HEADERS*>(image + dos->e_lfanew);
    nt->OptionalHeader.SizeOfImage = static_cast<DWORD>(imageSize);
    HMODULE engine = reinterpret_cast<HMODULE>(image);
    std::memcpy(image + bm::kSoundEntryRva, bm::kSoundEntry, sizeof(bm::kSoundEntry));
    std::memcpy(image + bm::kSoundScaleRva, bm::kSoundScale, sizeof(bm::kSoundScale));
    if (bm::SoundState(engine) != bm::SITE_STOCK || !bm::PrepareSoundStub(engine))
        return 2;
    unsigned char jump[sizeof(bm::kSoundEntry)];
    bm::BuildSoundJump(jump, reinterpret_cast<uintptr_t>(bm::g_soundStub));
    if (!bm::WriteBytes(image + bm::kSoundEntryRva, jump, sizeof(jump)) ||
        bm::SoundState(engine) != bm::SITE_PATCHED)
        return 3;
    unsigned char damaged = 0;
    if (!bm::WriteBytes(bm::g_soundStub, &damaged, 1) || bm::SoundState(engine) != bm::SITE_OTHER)
        return 4;
    unsigned char expected[64];
    const size_t length = bm::BuildSoundTrampoline(expected, reinterpret_cast<uintptr_t>(engine));
    if (!bm::WriteBytes(bm::g_soundStub, expected, length) || bm::SoundState(engine) != bm::SITE_PATCHED)
        return 5;
    if (!bm::WriteBytes(image + bm::kSoundEntryRva, bm::kSoundEntry, sizeof(bm::kSoundEntry)) ||
        bm::SoundState(engine) != bm::SITE_STOCK)
        return 6;
    image[bm::kSoundScaleRva] ^= 1;
    if (bm::SoundState(engine) != bm::SITE_OTHER)
        return 7;
    VirtualFree(bm::g_soundStub, 0, MEM_RELEASE);
    VirtualFree(image, 0, MEM_RELEASE);
    std::puts("Sound hook stock, patched, tampered and restored states: OK");
    return 0;
}