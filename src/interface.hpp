#pragma once

#include <array>
#include <string_view>
#include <utility>


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


template<typename T, typename... Args>
constexpr auto make_array(Args&&... args) -> std::array<T, sizeof...(Args)>
{
    return { std::forward<Args>(args)... };
}


/**
 * \brief Base class imitating the server plugin callback interface.
 *
 * See SDK's \c IServerPluginCallbacks.
*/
class ServerPluginCallbacks
{
public:
    static constexpr std::array COMPATIBLE_VERSIONS = make_array<std::string_view>(
        "ISERVERPLUGINCALLBACKS001",
        "ISERVERPLUGINCALLBACKS002",
        "ISERVERPLUGINCALLBACKS003"
    );

    /**
     * \brief Instance for \c CreateInterface.
    */
    static inline ServerPluginCallbacks *Instance = nullptr;

    ServerPluginCallbacks()
    {
        if (Instance == nullptr)
            Instance = this;
    }

    ~ServerPluginCallbacks()
    {
        Instance = nullptr;
    }

public:

    // ISERVERPLUGINCALLBACKS001

    virtual bool Load(CreateInterfaceFn *interface_factory, CreateInterfaceFn *game_server_factory) = 0;

    virtual void Unload() = 0;

    virtual void Pause() {}

    virtual void UnPause() {}

    virtual const char *GetPluginDescription() = 0;

    virtual void LevelInit(char const *map_name) {}

    virtual void ServerActivate(edict_t *edict_list, int edict_count, int client_max) {}

    virtual void GameFrame(bool simulating) {}

    virtual void LevelShutdown(void) {}

    virtual void ClientActive(edict_t *entity) {}

    virtual void ClientDisconnect(edict_t *entity) {}

    virtual void ClientPutInServer(edict_t *entity, char const *player_name) {}

    virtual void SetCommandClient(int index) {}

    virtual void ClientSettingsChanged(edict_t *edict) {}

    virtual PluginResult ClientConnect(bool *allow_connect, edict_t *entity, const char *name, const char *address, char *reject, int max_reject_length) { return PluginResult::CONTINUE; }

    virtual PluginResult ClientCommand(edict_t *entity, const CCommand &args) { return PluginResult::CONTINUE; }

    virtual PluginResult NetworkIDValidated(const char *user_name, const char *network_id) { return PluginResult::CONTINUE; }

    // ISERVERPLUGINCALLBACKS002

    virtual void OnQueryCvarValueFinished(int cookie, edict_t *player_entity, int status, const char *cvar_name, const char *cvar_value) {}

    // ISERVERPLUGINCALLBACKS003

    virtual void OnEdictAllocated(edict_t *edict) {}

    virtual void OnEdictFreed(const edict_t *edict) {}
};
