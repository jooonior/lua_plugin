#pragma once

#if defined(_WIN32)
 #define INTERFACE extern "C" __declspec(dllexport)
 #define DYNAMIC_LIBRARY(Name) Name ".dll"
#else
 #define INTERFACE extern "C" __attribute__((visibility("default")))
 #define DYNAMIC_LIBRARY(Name) "lib" Name ".so"
#endif

void *GetModuleHandle(const char *module_name);

void *GetSymbolAddress(void *module_handle, const char *symbol_name);

template<typename T>
T *GetSymbolAddress(void *module_handle, const char *symbol_name)
{
    return reinterpret_cast<T *>(GetSymbolAddress(module_handle, symbol_name));
}
