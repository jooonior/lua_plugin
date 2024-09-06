#include "engine.hpp"
#include "interface.hpp"
#include "L.hpp"

#include <lua.hpp>
#include <whereami.h>

#include <filesystem>
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
    bool TryCallLuaMethod(const char *method, int retc, Args&&... args)
    {
        if (L == nullptr)
            return false;

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
        // Get path to this module.

        int length = wai_getModulePath(nullptr, 0, nullptr);
        if (length < 0)
        {
            // Nothing we can do...
            return;
        }

        _path.resize(length);
        wai_getModulePath(_path.data(), _path.capacity(), nullptr);

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

        std::error_code error;
        if (!std::filesystem::is_regular_file(script_path, error))
        {
            // Remove `.lua` extension.
            script_path.resize(script_path.length() - 4);

            script_path.append("/init.lua");

            if (!std::filesystem::is_regular_file(script_path, error))
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

        if (!TryCallLuaMethod("Load", LUA_MULTRET, interface_factory, game_server_factory))
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

        TryCallLuaMethod("Unload", 0);

        lua_close(L);
        L = nullptr;
    }

    const char *GetPluginDescription() override
    {
        if (TryCallLuaMethod("GetPluginDescription", 1))
        {
            const char *description = lua_tostring(L, -1);
            if (description != nullptr)
                _description = description;

            lua_pop(L, 1);
        }

        return _description.c_str();
    }

    void Pause() override
    {
        TryCallLuaMethod("Pause", 0);
    }

    void UnPause() override
    {
        TryCallLuaMethod("UnPause", 0);
    }

    void LevelInit(char const *map_name) override
    {
        TryCallLuaMethod("LevelInit", 0);
    }

    void ServerActivate(edict_t *edict_list, int edict_count, int client_max) override
    {
        TryCallLuaMethod("ServerActivate", 0, edict_list, edict_count, client_max);
    }

    void GameFrame(bool simulating) override
    {
        TryCallLuaMethod("GameFrame", 0, simulating);
    }

    void LevelShutdown() override
    {
        TryCallLuaMethod("LevelShutdown", 0);
    }

    void ClientActive(edict_t *entity) override
    {
        TryCallLuaMethod("ClientActive", 0, entity);
    }

    void ClientDisconnect(edict_t *entity) override
    {
        TryCallLuaMethod("ClientDisconnect", 0, entity);
    }

    void ClientPutInServer(edict_t *entity, char const *player_name) override
    {
        TryCallLuaMethod("ClientPutInServer", 0, entity, player_name);
    }

    void SetCommandClient(int index) override
    {
        TryCallLuaMethod("SetCommandClient", 0, index);
    }

    void ClientSettingsChanged(edict_t *edict) override
    {
        TryCallLuaMethod("ClientSettingsChanged", 0, edict);
    }

    PluginResult ClientConnect(bool *allow_connect, edict_t *entity, const char *name, const char *address, char *reject, int max_reject_length) override
    {
        if (TryCallLuaMethod("ClientConnect", 1, allow_connect, entity, name, address, reject, max_reject_length))
        {
            auto result = static_cast<PluginResult>(lua_tointeger(L, -1));
            lua_pop(L, 1);

            if (IsValidPluginResult(result))
                return result;

            PluginWarn("Invalid Plugin::ClientConnect result: %i\n", result);
        }

        return PluginResult::CONTINUE;
    }

    PluginResult ClientCommand(edict_t *entity, const CCommand &args) override
    {
        if (TryCallLuaMethod("ClientCommand", 1, entity, &args))
        {
            auto result = static_cast<PluginResult>(lua_tointeger(L, -1));
            lua_pop(L, 1);

            if (IsValidPluginResult(result))
                return result;

            PluginWarn("Invalid Plugin::ClientCommand result: %i\n", result);
        }

        return PluginResult::CONTINUE;
    }

    PluginResult NetworkIDValidated(const char *user_name, const char *network_id) override
    {
        if (TryCallLuaMethod("NetworkIDValidated", 1, user_name, network_id))
        {
            auto result = static_cast<PluginResult>(lua_tointeger(L, -1));
            lua_pop(L, 1);

            if (IsValidPluginResult(result))
                return result;

            PluginWarn("Invalid Plugin::NetworkIDValidated result: %i\n", result);
        }

        return PluginResult::CONTINUE;
    }

    void OnQueryCvarValueFinished(int cookie, edict_t *player_entity, int status, const char *cvar_name, const char *cvar_value) override
    {
        TryCallLuaMethod("OnQueryCvarValueFinished", 0, cookie, player_entity, status, cvar_name, cvar_value);
    }

    void OnEdictAllocated(edict_t *edict) override
    {
        TryCallLuaMethod("OnEdictAllocated", 0, edict);
    }

    void OnEdictFreed(const edict_t *edict) override
    {
        TryCallLuaMethod("OnEdictFreed", 0, edict);
    }
};


// Plugin instance returned by the `CreateInterface` entry point.
static Plugin PluginInstance;
