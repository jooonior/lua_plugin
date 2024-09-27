#pragma once

#include <string_view>


using CreateInterfaceFn = void *(const char *, int *);

struct edict_t;
class CCommand;


enum class PluginResult
{
    CONTINUE = 0,
    OVERRIDE,
    STOP,
};


inline bool IsValidPluginResult(PluginResult result)
{
    switch (result)
    {
    case PluginResult::CONTINUE:
    case PluginResult::OVERRIDE:
    case PluginResult::STOP:
        return true;
    }

    return false;
}

inline bool IsValidPluginResult(int result)
{
    return IsValidPluginResult(static_cast<PluginResult>(result));
}


/**
 * @brief Common prefix to \c IServerPluginCallbacks_v1 and \c IServerPluginCallbacks_v2.
 */
struct IServerPluginCallbacks_Common
{
    virtual bool Load(CreateInterfaceFn *interface_factory, CreateInterfaceFn *game_server_factory) = 0;

    virtual void Unload() = 0;

    virtual void Pause() = 0;

    virtual void UnPause() = 0;

    virtual const char *GetPluginDescription() = 0;

    virtual void LevelInit(char const *map_name) = 0;

    virtual void ServerActivate(edict_t *edict_list, int edict_count, int client_max) = 0;

    virtual void GameFrame(bool simulating) = 0;

    virtual void LevelShutdown(void) = 0;

    virtual void ClientActive(edict_t *entity) = 0;

    virtual void ClientDisconnect(edict_t *entity) = 0;

    virtual void ClientPutInServer(edict_t *entity, char const *player_name) = 0;

    virtual void SetCommandClient(int index) = 0;

    virtual void ClientSettingsChanged(edict_t *edict) = 0;

    virtual PluginResult ClientConnect(bool *allow_connect, edict_t *entity, const char *name, const char *address, char *reject, int max_reject_length) = 0;
};

struct IServerPluginCallbacks_v1 : IServerPluginCallbacks_Common
{
    static constexpr std::string_view INTERFACE_VERSION = "ISERVERPLUGINCALLBACKS001";

    // This function is different between versions 1 and 2.
    virtual PluginResult ClientCommand_v1(edict_t *entity) = 0;

    virtual PluginResult NetworkIDValidated(const char *user_name, const char *network_id) = 0;
};

struct IServerPluginCallbacks_v2 : IServerPluginCallbacks_Common
{
    static constexpr std::string_view INTERFACE_VERSION = "ISERVERPLUGINCALLBACKS002";

    // This function is different between versions 1 and 2.
    virtual PluginResult ClientCommand_v2(edict_t *entity, const CCommand &args) = 0;

    virtual PluginResult NetworkIDValidated(const char *user_name, const char *network_id) = 0;

    virtual void OnQueryCvarValueFinished(int cookie, edict_t *player_entity, int status, const char *cvar_name, const char *cvar_value) = 0;
};

struct IServerPluginCallbacks_v3 : IServerPluginCallbacks_v2
{
    static constexpr std::string_view INTERFACE_VERSION = "ISERVERPLUGINCALLBACKS003";

    virtual void OnEdictAllocated(edict_t *edict) = 0;

    virtual void OnEdictFreed(const edict_t *edict) = 0;
};

/**
 * @brief Like \c IServerPluginCallbacks_v3, but with an extra function in the middle.
 */
struct IServerPluginCallbacks_Portal2
{
    static constexpr std::string_view INTERFACE_VERSION = IServerPluginCallbacks_v3::INTERFACE_VERSION;

    virtual bool Load(CreateInterfaceFn *interface_factory, CreateInterfaceFn *game_server_factory) = 0;

    virtual void Unload() = 0;

    virtual void Pause() = 0;

    virtual void UnPause() = 0;

    virtual const char *GetPluginDescription() = 0;

    virtual void LevelInit(char const *map_name) = 0;

    virtual void ServerActivate(edict_t *edict_list, int edict_count, int client_max) = 0;

    virtual void GameFrame(bool simulating) = 0;

    virtual void LevelShutdown(void) = 0;

    virtual void ClientActive(edict_t *entity) = 0;

    // This added function makes it incompatible with IServerPluginCallbacks_v3.
    virtual void ClientFullyConnect(edict_t *entity) = 0;

    virtual void ClientDisconnect(edict_t *entity) = 0;

    virtual void ClientPutInServer(edict_t *entity, char const *player_name) = 0;

    virtual void SetCommandClient(int index) = 0;

    virtual void ClientSettingsChanged(edict_t *edict) = 0;

    virtual PluginResult ClientConnect(bool *allow_connect, edict_t *entity, const char *name, const char *address, char *reject, int max_reject_length) = 0;

    virtual PluginResult ClientCommand_v2(edict_t *entity, const CCommand &args) = 0;

    virtual PluginResult NetworkIDValidated(const char *user_name, const char *network_id) = 0;

    virtual void OnQueryCvarValueFinished(int cookie, edict_t *player_entity, int status, const char *cvar_name, const char *cvar_value) = 0;

    virtual void OnEdictAllocated(edict_t *edict) = 0;

    virtual void OnEdictFreed(const edict_t *edict) = 0;
};


/**
 * @brief Wraps \c ServerPlugin in an interface compatible with specified \c IServerPluginCallbacks version.
 * @tparam ServerPlugin
 * @tparam IServerPluginCallbacks
 */
template<typename ServerPlugin, typename IServerPluginCallbacks>
struct ServerPluginRouter : IServerPluginCallbacks
{
    ServerPlugin plugin;

    ServerPluginRouter() : plugin(IServerPluginCallbacks::INTERFACE_VERSION) {}

    // Defines all possible `IServerPluginCallbacks` functions. Those that override inherited
    // ones are laid out according to the base vtable, the rest ends up unused at the end.

    virtual bool Load(CreateInterfaceFn *interface_factory, CreateInterfaceFn *game_server_factory)
    {
        return plugin.Load(interface_factory, game_server_factory);
    }

    virtual void Unload()
    {
        plugin.Unload();
    }

    virtual void Pause()
    {
        plugin.Pause();
    }

    virtual void UnPause()
    {
        plugin.UnPause();
    }

    virtual const char *GetPluginDescription()
    {
        return plugin.GetPluginDescription();
    }

    virtual void LevelInit(char const *map_name)
    {
        plugin.LevelInit(map_name);
    }

    virtual void ServerActivate(edict_t *edict_list, int edict_count, int client_max)
    {
        plugin.ServerActivate(edict_list, edict_count, client_max);
    }

    virtual void GameFrame(bool simulating)
    {
        plugin.GameFrame(simulating);
    }

    virtual void LevelShutdown(void)
    {
        plugin.LevelShutdown();
    }

    virtual void ClientActive(edict_t *entity)
    {
        plugin.ClientActive(entity);
    }

    virtual void ClientFullyConnect(edict_t *entity)
    {
        plugin.ClientFullyConnect(entity);
    }

    virtual void ClientDisconnect(edict_t *entity)
    {
        plugin.ClientDisconnect(entity);
    }

    virtual void ClientPutInServer(edict_t *entity, char const *player_name)
    {
        plugin.ClientPutInServer(entity, player_name);
    }

    virtual void SetCommandClient(int index)
    {
        plugin.SetCommandClient(index);
    }

    virtual void ClientSettingsChanged(edict_t *edict)
    {
        plugin.ClientSettingsChanged(edict);
    }

    virtual PluginResult ClientConnect(bool *allow_connect, edict_t *entity, const char *name, const char *address, char *reject, int max_reject_length)
    {
        return plugin.ClientConnect(allow_connect, entity, name, address, reject, max_reject_length);
    }

    virtual PluginResult ClientCommand_v1(edict_t *entity)
    {
        return plugin.ClientCommand_v1(entity);
    }

    virtual PluginResult ClientCommand_v2(edict_t *entity, const CCommand &args)
    {
        return plugin.ClientCommand_v2(entity, args);
    }

    virtual PluginResult NetworkIDValidated(const char *user_name, const char *network_id)
    {
        return plugin.NetworkIDValidated(user_name, network_id);
    }

    virtual void OnQueryCvarValueFinished(int cookie, edict_t *player_entity, int status, const char *cvar_name, const char *cvar_value)
    {
        plugin.OnQueryCvarValueFinished(cookie, player_entity, status, cvar_name, cvar_value);
    }

    virtual void OnEdictAllocated(edict_t *edict)
    {
        plugin.OnEdictAllocated(edict);
    }

    virtual void OnEdictFreed(const edict_t *edict)
    {
        plugin.OnEdictFreed(edict);
    }
};
