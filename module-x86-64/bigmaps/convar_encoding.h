#pragma once

#include <cstdint>
#include <cstring>

namespace bm
{

inline std::uint32_t EncodeConVarFloat(float value, std::uintptr_t object)
{
    std::uint32_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    return bits ^ static_cast<std::uint32_t>(object);
}

inline std::uint32_t EncodeConVarInt(std::int32_t value, std::uintptr_t object)
{
    return static_cast<std::uint32_t>(value) ^ static_cast<std::uint32_t>(object);
}

}  // namespace bm