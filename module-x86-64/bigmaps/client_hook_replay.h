#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace bm
{

inline std::size_t BuildClientHookReplay(unsigned char* output, const unsigned char* original,
                                         std::uintptr_t engineBase)
{
    std::memcpy(output, original, 9);
    output[9] = 0x48;
    output[10] = 0xB8;
    const std::uint64_t address = static_cast<std::uint64_t>(engineBase + 0x004F87D8);
    std::memcpy(output + 11, &address, sizeof(address));
    output[19] = 0x48;
    output[20] = 0x8B;
    output[21] = 0x00;
    return 22;
}

}  // namespace bm