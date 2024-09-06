#include "platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>


// `Windows.h` defines this as a macro.
#undef GetModuleHandle

void *GetModuleHandle(const char *module_name)
{
    return GetModuleHandleA(module_name);
}

void *GetSymbolAddress(void *module_handle, const char *symbol_name)
{
    return GetProcAddress(reinterpret_cast<HMODULE>(module_handle), symbol_name);
}
