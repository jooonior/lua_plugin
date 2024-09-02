#include "engine.hpp"
#include "interface.hpp"
#include "platform.hpp"
#include "L.hpp"

#include <lua.hpp>

#include <string>
#include <utility>


class Plugin : public ServerPluginCallbacks
{
private:
    lua_State *L = nullptr;
    std::string name;

    template<typename... Args>
    void PluginPrint(const char *format, Args&&... args)
    {
        Print("[%s] ", name.c_str());
        Print(format, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void PluginWarn(const char *format, Args&&... args)
    {
        Warn("[%s] ", name.c_str());
        Warn(format, std::forward<Args>(args)...);
    }

public:
    bool Load(CreateInterfaceFn *interface_factory, CreateInterfaceFn *game_server_factory) override
    {
        if (!ConnectEnginePrintFunctions())
            return false;

        // Get path to this DLL.
        std::string module_path = GetPathToThisModule();
        if (module_path.empty())
            return false;

        // Strip file extension.
        auto last_dot = module_path.find_last_of(".");
        if (last_dot != module_path.npos)
            module_path.resize(last_dot);

        // Get file name.
        auto filename_start = module_path.find_last_of("\\/") + 1;  // overflows to 0 if not found
        name = module_path.substr(filename_start);

        // Find Lua script or module matching the plugin's name.

        std::string script_path = module_path + ".lua";
        module_path.resize(filename_start);

        if (!FileExists(script_path.c_str()))
        {
            script_path.resize(last_dot);
            script_path.append("/init.lua");

            if (!FileExists(script_path.c_str()))
            {
                PluginPrint(
                    "Lua entry point not found. Neither \"%s.lua\" nor \"%s/init.lua\" were found inside \"%s\".\n",
                    name.c_str(), name.c_str(), module_path.c_str()
                );

                return false;
            }
        }

        // Initialize Lua and run the script.

        L = luaL_newstate();
        if (L == nullptr)
        {
            PluginWarn("Could not create Lua state.\n");
            return false;
        }

        luaL_openlibs(L);

        L_SetGlobalFunction(L, "print", &L_Print<Print>);
        L_SetGlobalFunction(L, "warn", &L_Print<Warn>);

        L_SetPackagePath(L, module_path.c_str());

        if (L_RunFile(L, script_path.c_str(), LUA_MULTRET))
        {
            // No return value from script means success.
            if (lua_gettop(L) == 0)
                return true;

            // Pop return value(s).
            bool success = lua_toboolean(L, 0);
            lua_settop(L, 0);

            if (success)
                return true;
        }

        lua_close(L);
        L = nullptr;

        return false;
    }

    void Unload() override
    {
        // `Unload` is called even when `Load` fails.
        if (L == nullptr)
            return;

        lua_close(L);
        L = nullptr;
    }

    const char *GetPluginDescription() override
    {
        return name.c_str();
    }
};


// Plugin instance returned by the `CreateInterface` entry point.
static Plugin PluginInstance;
