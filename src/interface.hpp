#pragma once

using CreateInterfaceFn = void *(const char *, int *);

struct edict_t;
class CCommand;

enum class PluginResult
{
    CONTINUE = 0,
    OVERRIDE,
    STOP,
};


/**
 * \brief Base class imitating the server plugin callback interface.
 *
 * See SDK's \c IServerPluginCallbacks.
*/
class ServerPluginCallbacks
{
public:
    static constexpr const char *INTERFACE_VERSION = "ISERVERPLUGINCALLBACKS001";

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
};
