#include "interface.hpp"


class Plugin : public ServerPluginCallbacks
{
public:
    bool Load(CreateInterfaceFn *interface_factory, CreateInterfaceFn *game_server_factory) override
    {
        return true;
    }

    void Unload() override
    {
    }

    const char *GetPluginDescription() override
    {
        return "Plugin";
    }
};


// Plugin instance returned by the `CreateInterface` entry point.
static Plugin PluginInstance;
