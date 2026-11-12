#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace bm
{

inline void BuildSoundJump(unsigned char* output, std::uintptr_t destination)
{
    const unsigned char jump[] = {0xFF, 0x25, 0, 0, 0, 0};
    const std::uint64_t address = static_cast<std::uint64_t>(destination);
    std::memcpy(output, jump, sizeof(jump));
    std::memcpy(output + sizeof(jump), &address, sizeof(address));
    std::memset(output + sizeof(jump) + sizeof(address), 0x90, 3);
}

inline std::size_t BuildSoundTrampoline(unsigned char* output, std::uintptr_t engineBase)
{
    unsigned char* current = output;
    const unsigned char prefix[] = {0x41, 0xFF, 0xC5, 0x50, 0x48, 0xB8};
    const unsigned char middle[] = {0xF3, 0x0F, 0x10, 0x08, 0x58, 0x45, 0x85, 0xC9, 0x75, 0x0E};
    const unsigned char jump[] = {0xFF, 0x25, 0, 0, 0, 0};
    auto append = [&](const void* bytes, std::size_t size) {
        std::memcpy(current, bytes, size);
        current += size;
    };
    auto address = [&](std::uintptr_t rva) {
        const std::uint64_t value = static_cast<std::uint64_t>(engineBase + rva);
        append(&value, sizeof(value));
    };
    append(prefix, sizeof(prefix));
    address(0x0039D08C);
    append(middle, sizeof(middle));
    append(jump, sizeof(jump));
    address(0x00211D6D);
    append(jump, sizeof(jump));
    address(0x00211C5A);
    return static_cast<std::size_t>(current - output);
}

}  // namespace bm