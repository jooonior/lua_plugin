#include "platform.hpp"

#if defined(_WIN32) //============= Windows ===================================#

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

#elif defined(__linux__) //========= Linux ====================================#

#include <dlfcn.h>


void *GetModuleHandle(const char *module_name)
{
    void *handle = dlopen(module_name, RTLD_NOW | RTLD_NOLOAD);

    if (handle == nullptr)
        return nullptr;

    // Every successful `dlopen` needs a matching `dlclose`.
    dlclose(handle);

    return handle;
}

void *GetSymbolAddress(void *module_handle, const char *symbol_name)
{
    return dlsym(module_handle, symbol_name);
}

#else //=======================================================================#

#error "Platform not supported"

#endif
