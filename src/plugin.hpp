#pragma once

#include "engine.hpp"
#include "interface.hpp"

// #include <lua.hpp>
struct lua_State;

#include <string>


/**
 * @brief Plugin implementation. Must cover functions from all \c IServerPluginCallbacks versions.
 */
struct Plugin
{
private:
    lua_State *L = nullptr;
    std::string _path;
    std::string _name;
    std::string _description;

protected:
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

public:
    Plugin();

    virtual bool Load(CreateInterfaceFn *interface_factory, CreateInterfaceFn *game_server_factory);

    virtual void Unload();

    virtual void Pause();

    virtual void UnPause();

    virtual const char *GetPluginDescription();

    virtual void LevelInit(char const *map_name);

    virtual void ServerActivate(edict_t *edict_list, int edict_count, int client_max);

    virtual void GameFrame(bool simulating);

    virtual void LevelShutdown(void);

    virtual void ClientActive(edict_t *entity);

    virtual void ClientFullyConnect(edict_t *entity);

    virtual void ClientDisconnect(edict_t *entity);

    virtual void ClientPutInServer(edict_t *entity, char const *player_name);

    virtual void SetCommandClient(int index);

    virtual void ClientSettingsChanged(edict_t *edict);

    virtual PluginResult ClientConnect(bool *allow_connect, edict_t *entity, const char *name, const char *address, char *reject, int max_reject_length);

    virtual PluginResult ClientCommand(edict_t *entity, const CCommand &args);

    virtual PluginResult NetworkIDValidated(const char *user_name, const char *network_id);

    virtual void OnQueryCvarValueFinished(int cookie, edict_t *player_entity, int status, const char *cvar_name, const char *cvar_value);

    virtual void OnEdictAllocated(edict_t *edict);

    virtual void OnEdictFreed(const edict_t *edict);
};
