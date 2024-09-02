#include "engine.hpp"
#include "interface.hpp"
#include "platform.hpp"
#include "L.hpp"

#include <lua.hpp>

#include <string>
#include <utility>


template<typename F>
struct defer
{
private:
    F _fn;
    bool _cancelled = false;

public:
    defer(F &&fn = {}) : _fn{ fn } {}

    ~defer()
    {
        if (!_cancelled)
            _fn();
    }

    void cancel()
    {
        _cancelled = true;
    }
};


class Plugin : public ServerPluginCallbacks
{
private:
    lua_State *L = nullptr;
    std::string _path;
    std::string _name;
    std::string _description;

    template<typename... Args>
    void PluginPrint(const char *format, Args&&... args)
    {
        Print("[%s] ", _name.c_str());
        Print(format, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void PluginWarn(const char *format, Args&&... args)
    {
        Warn("[%s] ", _name.c_str());
        Warn(format, std::forward<Args>(args)...);
    }

    template<typename... Args>
    bool CallLuaMethod(const char *method, int retc, Args&&... args)
    {
        lua_getfield(L, -1, method);

        if (!lua_isfunction(L, -1))
        {
            lua_pop(L, 1);
            lua_settop(L, lua_gettop(L) + retc);
            return true;
        }

        lua_pushvalue(L, -2);  // the `self` argument
        L_Push(L, std::forward<Args>(args)...);

        return L_TryCall(L, 1 + sizeof...(args), retc);
    }

public:
    Plugin()
    {
        // Get path to this DLL.
        _path = GetPathToThisModule();
        if (_path.empty())
            return;

        // Strip file extension.
        auto last_dot = _path.find_last_of(".");
        if (last_dot != _path.npos)
            _path.resize(last_dot);

        // Get file name.
        auto filename_start = _path.find_last_of("\\/") + 1;  // overflows to 0 if not found
        _name = _path.substr(filename_start);

        // Strip file name.
        _path.resize(filename_start);

        // Use name as default description.
        _description = _name;
    }

    bool Load(CreateInterfaceFn *interface_factory, CreateInterfaceFn *game_server_factory) override
    {
        if (!ConnectEnginePrintFunctions())
            return false;

        if (_name.empty())
            return false;

        // Find Lua script or module matching the plugin's name.

        std::string script_path = _path;
        script_path.append(_name).append(".lua");

        if (!FileExists(script_path.c_str()))
        {
            // Remove `.lua` extension.
            script_path.resize(script_path.length() - 4);

            script_path.append("/init.lua");

            if (!FileExists(script_path.c_str()))
            {
                PluginPrint(
                    "Lua entry point not found. Neither \"%s.lua\" nor \"%s/init.lua\" were found inside \"%s\".\n",
                    _name.c_str(), _name.c_str(), _path.c_str()
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

        defer release_lua_state([&]() {
            lua_close(L);
            L = nullptr;
        });

        luaL_openlibs(L);

        L_SetGlobalFunction(L, "print", &L_Print<Print>);
        L_SetGlobalFunction(L, "warn", &L_Print<Warn>);

        L_SetPackagePath(L, _path.c_str());

        if (!L_RunFile(L, script_path.c_str(), 1))
            return false;

        if (!lua_istable(L, 1))
        {
            PluginPrint("Lua entry point \"%s\" did not return a table.\n", script_path.c_str());
            return false;
        }

        if (!CallLuaMethod("Load", LUA_MULTRET, interface_factory, game_server_factory))
            return false;

        // Treat no return value as success.
        if (lua_gettop(L) == 1 || lua_toboolean(L, 2))
        {
            // Leave only the plugin table.
            lua_settop(L, 1);

            release_lua_state.cancel();
            return true;
        }

        return false;
    }

    void Unload() override
    {
        // `Unload` is called even when `Load` fails.
        if (L == nullptr)
            return;

        CallLuaMethod("Unload", 0);

        lua_close(L);
        L = nullptr;
    }

    const char *GetPluginDescription() override
    {
        if (L != nullptr && CallLuaMethod("GetPluginDescription", 1))
        {
            const char *description = lua_tostring(L, -1);
            if (description != nullptr)
                _description = description;

            lua_pop(L, 1);
        }

        return _description.c_str();
    }
};


// Plugin instance returned by the `CreateInterface` entry point.
static Plugin PluginInstance;
