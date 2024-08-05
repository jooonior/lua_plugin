#include "interface.hpp"

#include <cstring>
#include <type_traits>


extern "C" __declspec(dllexport)
void *CreateInterface(const char *name, int *return_code)
{
    void *interface_ptr = nullptr;

    if (strcmp(name, ServerPluginCallbacks::INTERFACE_VERSION) == 0)
    {
        interface_ptr = ServerPluginCallbacks::Instance;
    }

    if (return_code != nullptr)
    {
        *return_code = interface_ptr != nullptr;
    }

    return interface_ptr;
}


static_assert(std::is_same<decltype(CreateInterface), CreateInterfaceFn>{});
