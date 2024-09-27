#ifndef _WIN32
#error "Delay load hook is only available on Windows"
#endif

#include "platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <delayimp.h>

#include <string>


HMODULE LoadLibraryRelative(const char *name)
{
    const char *this_module_path = GetModulePath();
    if (this_module_path == nullptr)
        return nullptr;

    std::string path(this_module_path);
    auto filename_start = path.find_last_of("\\/") + 1;  // overflows to 0 if not found
    path.resize(filename_start);
    path.append(name);

    return LoadLibraryA(path.data());
}

FARPROC WINAPI DelayHook(unsigned dliNotify, PDelayLoadInfo pdli)
{
    switch (dliNotify)
    {
    case dliNotePreLoadLibrary:
        return reinterpret_cast<FARPROC>(LoadLibraryRelative(pdli->szDll));

    default:
        return nullptr;
    }
}

extern "C"
{
    const PfnDliHook __pfnDliNotifyHook2 = DelayHook;
    const PfnDliHook __pfnDliFailureHook2 = DelayHook;
}
