#pragma once

#include <string>

#if defined(_WIN32)
 #define INTERFACE extern "C" __declspec(dllexport)
 #define DYNAMIC_LIBRARY_PREFIX
 #define DYNAMIC_LIBRARY_EXTENSION ".dll"
#else
 #define INTERFACE extern "C" __attribute__((visibility("default")))
 #define DYNAMIC_LIBRARY_PREFIX "lib"
 #define DYNAMIC_LIBRARY_EXTENSION ".so"
#endif

#define DYNAMIC_LIBRARY(Name) DYNAMIC_LIBRARY_PREFIX Name DYNAMIC_LIBRARY_EXTENSION

void *GetModuleHandle(const char *module_name);

void *GetSymbolAddress(void *module_handle, const char *symbol_name);

template<typename T>
T *GetSymbolAddress(void *module_handle, const char *symbol_name)
{
    return reinterpret_cast<T *>(GetSymbolAddress(module_handle, symbol_name));
}

std::string GetExecutableName();
