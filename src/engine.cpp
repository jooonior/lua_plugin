#include "engine.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>


bool ConnectEnginePrintFunctions()
{
    auto *tier0 = GetModuleHandleA("tier0.dll");

    if (tier0 == nullptr)
        return false;

    Print = reinterpret_cast<PrintFn_t *>(GetProcAddress(tier0, "Msg"));
    if (Print == nullptr)
        return false;

    Warn = reinterpret_cast<PrintFn_t *>(GetProcAddress(tier0, "Warning"));
    if (Warn == nullptr)
        return false;

    return true;
}
