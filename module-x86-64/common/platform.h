// Entry point of the shared layer: standard headers, then the OS layer for this build, which pulls in the
// target data (../targets). Everything is header-only; each module is built as one translation unit.
#pragma once

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
#include "platform_win.h"
#elif defined(__linux__)
#include "platform_linux.h"
#else
#error "bigmaps: unsupported operating system"
#endif

namespace bm
{

// World extent limit of the stock engine, and the expanded one this module enables.
const float kStockExtent = 16384.0f;
const float kLargeExtent = 32768.0f;

}  // namespace bm
