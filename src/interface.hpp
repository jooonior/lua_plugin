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

    virtual PluginResult ClientCommand(edict_t *entity, const CCommand &args) = 0;

    virtual PluginResult NetworkIDValidated(const char *user_name, const char *network_id) = 0;

    // ISERVERPLUGINCALLBACKS002

    virtual void OnQueryCvarValueFinished(int cookie, edict_t *player_entity, int status, const char *cvar_name, const char *cvar_value) = 0;

    // ISERVERPLUGINCALLBACKS003

    virtual void OnEdictAllocated(edict_t *edict) = 0;

    virtual void OnEdictFreed(const edict_t *edict) = 0;
};
