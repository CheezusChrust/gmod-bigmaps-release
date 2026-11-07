// Console output through tier0's Msg, so messages appear in the game / server console and its log.
#pragma once

#include "platform.h"

namespace bm
{

typedef void (*MsgFn)(const char* fmt, ...);
inline MsgFn& MsgPtr()
{
    static MsgFn fn = nullptr;
    return fn;
}

inline void InitConsole()
{
#ifdef _WIN32
    if (HMODULE tier0 = GetModuleHandleA(kTier0))
        MsgPtr() = reinterpret_cast<MsgFn>(GetProcAddress(tier0, "Msg"));
#else
    if (void* h = dlopen(kTier0, RTLD_NOW | RTLD_NOLOAD))  // RTLD_NOLOAD: only if already loaded
    {
        MsgPtr() = reinterpret_cast<MsgFn>(dlsym(h, "Msg"));
        dlclose(h);
    }
#endif
}

// Formats locally and passes the result as "%s", so text is never interpreted as a format by Msg.
// Falls back to the debugger output (Windows) or stderr (Linux) before InitConsole or without tier0.
inline void Out(const char* fmt, ...)
{
    char buf[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    if (MsgPtr())
        MsgPtr()("%s", buf);
    else
#ifdef _WIN32
        OutputDebugStringA(buf);
#else
        fputs(buf, stderr);
#endif
}

}  // namespace bm
