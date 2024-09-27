#include "platform.hpp"

//============================== All Platforms ================================#

#include <whereami.h>


const char *GetModulePath()
{
    static std::string path;

    if (path.empty())
    {
        auto length = wai_getModulePath(nullptr, 0, nullptr);
        if (length < 0)
            return nullptr;

        path.resize(length);

        length = wai_getModulePath(path.data(), path.size(), nullptr);
        if (length < 0)
        {
            path.clear();
            return nullptr;
        }
    };

    return path.data();
}

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

std::string GetExecutableName()
{
    std::string name(MAX_PATH, 0);

    // Apparently this is how the API is supposed to be used.
    while (true)
    {
        auto length = GetModuleFileNameA(nullptr, name.data(), name.size());

        if (length < name.size())
        {
            name.resize(length);
            break;
        }

        name.resize(name.size() * 2);
    }

    // Keep only filename.
    auto last_slash = name.find_last_of("\\/");
    if (last_slash != name.npos)
    {
        name.erase(0, last_slash + 1);
    }

    return name;
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

extern char *program_invocation_short_name;

std::string GetExecutableName()
{
    return program_invocation_short_name;
}

#else //=======================================================================#

#error "Platform not supported"

#endif
