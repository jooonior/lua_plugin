#include "interface.hpp"

#include "engine.hpp"
#include "platform.hpp"
#include "plugin.hpp"

#include <cstring>
#include <type_traits>


template<typename IServerPluginCallbacks>
static void *GetPluginInstance()
{
    static ServerPluginRouter<Plugin, IServerPluginCallbacks> instance;
    return &instance;
}

static void *GetPluginInstance(const char *interface_version)
{
    std::string executable_name = GetExecutableName();

    if (executable_name.find("portal2") != executable_name.npos)
        return GetPluginInstance<IServerPluginCallbacks_Portal2>();

    if (interface_version == IServerPluginCallbacks_v1::INTERFACE_VERSION)
        return GetPluginInstance<IServerPluginCallbacks_v1>();

    if (interface_version == IServerPluginCallbacks_v2::INTERFACE_VERSION)
        return GetPluginInstance<IServerPluginCallbacks_v2>();

    if (interface_version == IServerPluginCallbacks_v3::INTERFACE_VERSION)
        return GetPluginInstance<IServerPluginCallbacks_v3>();

    return nullptr;
}


/**
 * \brief Interface factory that gets called by the engine.
*/
INTERFACE  // exported symbol
void *CreateInterface(const char *name, int *return_code)
{
    static bool are_print_functions_connected = ConnectEnginePrintFunctions();
    if (!are_print_functions_connected)
    {
        // Nothing we can do...
        return nullptr;
    }

    void *interface_ptr = GetPluginInstance(name);

    if (return_code != nullptr)
    {
        *return_code = interface_ptr != nullptr;
    }

    return interface_ptr;
}


static_assert(std::is_same<decltype(CreateInterface), CreateInterfaceFn>{});
