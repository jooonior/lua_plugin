#include "plugin.hpp"

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


template<typename... Args>
bool TryCallLuaMethod(lua_State *L, const char *method, int retc, Args&&... args)
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


Plugin::Plugin()
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

bool Plugin::Load(CreateInterfaceFn *interface_factory, CreateInterfaceFn *game_server_factory)
{
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

    if (!TryCallLuaMethod(L, "Load", LUA_MULTRET, interface_factory, game_server_factory))
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

void Plugin::Unload()
{
    // `Unload` is called even when `Load` fails.
    if (L == nullptr)
        return;

    TryCallLuaMethod(L, "Unload", 0);

    lua_close(L);
    L = nullptr;
}

const char *Plugin::GetPluginDescription()
{
    if (TryCallLuaMethod(L, "GetPluginDescription", 1))
    {
        const char *description = lua_tostring(L, -1);
        if (description != nullptr)
            _description = description;

        lua_pop(L, 1);
    }

    return _description.c_str();
}

void Plugin::Pause()
{
    TryCallLuaMethod(L, "Pause", 0);
}

void Plugin::UnPause()
{
    TryCallLuaMethod(L, "UnPause", 0);
}

void Plugin::LevelInit(char const *map_name)
{
    TryCallLuaMethod(L, "LevelInit", 0);
}

void Plugin::ServerActivate(edict_t *edict_list, int edict_count, int client_max)
{
    TryCallLuaMethod(L, "ServerActivate", 0, edict_list, edict_count, client_max);
}

void Plugin::GameFrame(bool simulating)
{
    TryCallLuaMethod(L, "GameFrame", 0, simulating);
}

void Plugin::LevelShutdown()
{
    TryCallLuaMethod(L, "LevelShutdown", 0);
}

void Plugin::ClientActive(edict_t *entity)
{
    TryCallLuaMethod(L, "ClientActive", 0, entity);
}

void Plugin::ClientFullyConnect(edict_t *entity)
{
    TryCallLuaMethod(L, "ClientFullyConnect", 0, entity);
}

void Plugin::ClientDisconnect(edict_t *entity)
{
    TryCallLuaMethod(L, "ClientDisconnect", 0, entity);
}

void Plugin::ClientPutInServer(edict_t *entity, char const *player_name)
{
    TryCallLuaMethod(L, "ClientPutInServer", 0, entity, player_name);
}

void Plugin::SetCommandClient(int index)
{
    TryCallLuaMethod(L, "SetCommandClient", 0, index);
}

void Plugin::ClientSettingsChanged(edict_t *edict)
{
    TryCallLuaMethod(L, "ClientSettingsChanged", 0, edict);
}

PluginResult Plugin::ClientConnect(bool *allow_connect, edict_t *entity, const char *name, const char *address, char *reject, int max_reject_length)
{
    if (TryCallLuaMethod(L, "ClientConnect", 1, allow_connect, entity, name, address, reject, max_reject_length))
    {
        auto result = static_cast<PluginResult>(lua_tointeger(L, -1));
        lua_pop(L, 1);

        if (IsValidPluginResult(result))
            return result;

        PluginWarn("Invalid Plugin::ClientConnect result: %i\n", result);
    }

    return PluginResult::CONTINUE;
}

PluginResult Plugin::ClientCommand_v1(edict_t *entity)
{
    if (TryCallLuaMethod(L, "ClientCommand", 1, entity))
    {
        auto result = static_cast<PluginResult>(lua_tointeger(L, -1));
        lua_pop(L, 1);

        if (IsValidPluginResult(result))
            return result;

        PluginWarn("Invalid Plugin::ClientCommand result: %i\n", result);
    }

    return PluginResult::CONTINUE;
}

PluginResult Plugin::ClientCommand_v2(edict_t *entity, const CCommand &args)
{
    if (TryCallLuaMethod(L, "ClientCommand", 1, entity, &args))
    {
        auto result = static_cast<PluginResult>(lua_tointeger(L, -1));
        lua_pop(L, 1);

        if (IsValidPluginResult(result))
            return result;

        PluginWarn("Invalid Plugin::ClientCommand result: %i\n", result);
    }

    return PluginResult::CONTINUE;
}

PluginResult Plugin::NetworkIDValidated(const char *user_name, const char *network_id)
{
    if (TryCallLuaMethod(L, "NetworkIDValidated", 1, user_name, network_id))
    {
        auto result = static_cast<PluginResult>(lua_tointeger(L, -1));
        lua_pop(L, 1);

        if (IsValidPluginResult(result))
            return result;

        PluginWarn("Invalid Plugin::NetworkIDValidated result: %i\n", result);
    }

    return PluginResult::CONTINUE;
}

void Plugin::OnQueryCvarValueFinished(int cookie, edict_t *player_entity, int status, const char *cvar_name, const char *cvar_value)
{
    TryCallLuaMethod(L, "OnQueryCvarValueFinished", 0, cookie, player_entity, status, cvar_name, cvar_value);
}

void Plugin::OnEdictAllocated(edict_t *edict)
{
    TryCallLuaMethod(L, "OnEdictAllocated", 0, edict);
}

void Plugin::OnEdictFreed(const edict_t *edict)
{
    TryCallLuaMethod(L, "OnEdictFreed", 0, edict);
}
