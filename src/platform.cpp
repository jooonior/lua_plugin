#include "platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>


// https://devblogs.microsoft.com/oldnewthing/20041025-00/?p=37483
extern "C" IMAGE_DOS_HEADER __ImageBase;
HMODULE ThisModule() { return reinterpret_cast<HMODULE>(&__ImageBase); }


std::string GetPathToThisModule()
{
    std::string path;
    path.resize(MAX_PATH);

    while (true)
    {
        auto buffer = const_cast<char *>(path.data());
        auto buffer_size = static_cast<DWORD>(path.capacity());
        auto chars_written = GetModuleFileNameA(ThisModule(), buffer, buffer_size);

        if (chars_written == 0)
        {
            path.resize(0);
            break;
        }

        if (chars_written < path.capacity())
        {
            path.resize(chars_written);
            break;
        }

        // Allocate more space and try again.
        path.resize(chars_written * 2);
    }

    return path;
}
