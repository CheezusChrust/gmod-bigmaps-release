// Machine-code emitter used to build the hook stubs and trampolines (both module hook sets).
#pragma once

#include "platform.h"

#include <initializer_list>

namespace bm
{

// Appends machine code to a buffer.
struct Emitter
{
    unsigned char* p;
    void Bytes(std::initializer_list<unsigned char> b) { for (unsigned char c : b) *p++ = c; }
    void Raw(const void* src, size_t n) { memcpy(p, src, n); p += n; }
    template <typename T> void Value(T v) { memcpy(p, &v, sizeof(v)); p += sizeof(v); }
};

}  // namespace bm
