#include "interface.hpp"

#include "platform.hpp"

#include <cstring>
#include <type_traits>


/**
 * \brief Interface factory that gets called by the engine.
*/
INTERFACE  // exported symbol
void *CreateInterface(const char *name, int *return_code)
{
    void *interface_ptr = nullptr;

    for (std::string_view version : ServerPluginCallbacks::COMPATIBLE_VERSIONS)
    {
        if (version == name)
        {
            interface_ptr = ServerPluginCallbacks::Instance;
            break;
        }
    }

    if (return_code != nullptr)
    {
        *return_code = interface_ptr != nullptr;
    }

    return interface_ptr;
}


static_assert(std::is_same<decltype(CreateInterface), CreateInterfaceFn>{});
