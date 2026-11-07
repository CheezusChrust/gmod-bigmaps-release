#pragma once

#include "sound_trampoline.h"

namespace bm
{

const unsigned kSoundEntryRva = 0x00211929;
const unsigned kSoundScaleRva = 0x0039CF7C;
const unsigned char kSoundEntry[] = {
    0xF3, 0x0F, 0x10, 0x0D, 0x4B, 0xB6, 0x18, 0x00, 0x45, 0x85, 0xC9, 0x0F, 0x84, 0x13, 0x01, 0x00, 0x00,
};
const unsigned char kSoundScale[] = {0x00, 0x00, 0x00, 0x41};
unsigned char* g_soundStub = nullptr;
HMODULE g_soundEngine = nullptr;

bool SoundStubOk(HMODULE engine)
{
    if (!g_soundStub || g_soundEngine != engine || !Readable(g_soundStub, 52))
        return false;
    unsigned char expected[64];
    const size_t size = BuildSoundTrampoline(expected, reinterpret_cast<uintptr_t>(engine));
    return size == 52 && !memcmp(g_soundStub, expected, size);
}

SiteState SoundState(HMODULE engine)
{
    unsigned char* base = reinterpret_cast<unsigned char*>(engine);
    if (kSoundEntryRva + sizeof(kSoundEntry) > ImageSize(engine) ||
        kSoundScaleRva + sizeof(kSoundScale) > ImageSize(engine) ||
        !Readable(base + kSoundEntryRva, sizeof(kSoundEntry)) ||
        !Readable(base + kSoundScaleRva, sizeof(kSoundScale)))
        return SITE_UNREADABLE;
    if (memcmp(base + kSoundScaleRva, kSoundScale, sizeof(kSoundScale)))
        return SITE_OTHER;
    if (!memcmp(base + kSoundEntryRva, kSoundEntry, sizeof(kSoundEntry)))
        return SITE_STOCK;
    if (!SoundStubOk(engine))
        return SITE_OTHER;
    unsigned char expected[sizeof(kSoundEntry)];
    BuildSoundJump(expected, reinterpret_cast<uintptr_t>(g_soundStub));
    return !memcmp(base + kSoundEntryRva, expected, sizeof(expected)) ? SITE_PATCHED : SITE_OTHER;
}

bool PrepareSoundStub(HMODULE engine)
{
    if (g_soundStub)
        return SoundStubOk(engine);
    unsigned char* memory = static_cast<unsigned char*>(VirtualAlloc(nullptr, 4096, MEM_COMMIT | MEM_RESERVE,
                                                                     PAGE_READWRITE));
    if (!memory)
        return false;
    const size_t size = BuildSoundTrampoline(memory, reinterpret_cast<uintptr_t>(engine));
    DWORD old;
    if (size != 52 || !VirtualProtect(memory, 4096, PAGE_EXECUTE_READ, &old))
    {
        VirtualFree(memory, 0, MEM_RELEASE);
        return false;
    }
    FlushInstructionCache(GetCurrentProcess(), memory, size);
    g_soundStub = memory;
    g_soundEngine = engine;
    return SoundStubOk(engine);
}

}  // namespace bm