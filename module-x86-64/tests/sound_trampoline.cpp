#include "../bigmaps/sound_trampoline.h"
#include "../bigmaps/client_hook_replay.h"
#include "../bigmaps/convar_encoding.h"

#include <cstdio>

int main()
{
    unsigned char code[64] = {};
    std::size_t length = bm::BuildSoundTrampoline(code, 0x180000000ull);
    const unsigned char expected[] = {
        0x41, 0xFF, 0xC5, 0x50, 0x48, 0xB8, 0x7C, 0xCF, 0x39, 0x80, 0x01, 0x00, 0x00, 0x00,
        0xF3, 0x0F, 0x10, 0x08, 0x58, 0x45, 0x85, 0xC9, 0x75, 0x0E,
        0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, 0x4D, 0x1A, 0x21, 0x80, 0x01, 0x00, 0x00, 0x00,
        0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, 0x3A, 0x19, 0x21, 0x80, 0x01, 0x00, 0x00, 0x00,
    };
    if (length != sizeof(expected) || std::memcmp(code, expected, sizeof(expected)))
        return 1;
    length = bm::BuildSoundTrampoline(code, 0x200000000ull);
    std::uint64_t target = 0;
    std::memcpy(&target, code + 6, sizeof(target));
    if (length != sizeof(expected) || target != 0x20039CF7Cull)
        return 2;
    std::memcpy(&target, code + 30, sizeof(target));
    if (target != 0x200211A4Dull)
        return 3;
    std::memcpy(&target, code + 44, sizeof(target));
    if (target != 0x20021193Aull)
        return 4;
    unsigned char jump[17];
    bm::BuildSoundJump(jump, 0x1234567887654321ull);
    const unsigned char expectedJump[] = {
        0xFF, 0x25, 0, 0, 0, 0, 0x21, 0x43, 0x65, 0x87, 0x78, 0x56, 0x34, 0x12, 0x90, 0x90, 0x90,
    };
    if (std::memcmp(jump, expectedJump, sizeof(expectedJump)))
        return 5;
    const unsigned char original[] = {
        0x40, 0x57, 0x48, 0x81, 0xEC, 0x20, 0x01, 0x00, 0x00, 0x48, 0x8B, 0x05, 0x58, 0xE4, 0x44, 0x00,
    };
    const unsigned char expectedReplay[] = {
        0x40, 0x57, 0x48, 0x81, 0xEC, 0x20, 0x01, 0x00, 0x00, 0x48, 0xB8,
        0xD8, 0x87, 0x4F, 0x80, 0x01, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x00,
    };
    length = bm::BuildClientHookReplay(code, original, 0x180000000ull);
    if (length != sizeof(expectedReplay) || std::memcmp(code, expectedReplay, sizeof(expectedReplay)))
        return 6;
    length = bm::BuildClientHookReplay(code, original, 0x200000000ull);
    std::memcpy(&target, code + 11, sizeof(target));
    if (length != sizeof(expectedReplay) || target != 0x2004F87D8ull)
        return 7;
    if (bm::EncodeConVarFloat(32768.0f, 0x1234567887654321ull) != 0xC0654321u ||
        bm::EncodeConVarInt(32768, 0x1234567887654321ull) != 0x8765C321u)
        return 8;
    std::puts("Sound and client trampoline bytes, relocation and ConVar encoding: OK");
    return 0;
}