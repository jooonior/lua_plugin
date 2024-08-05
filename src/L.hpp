#pragma once

#include "engine.hpp"

#include <lua.hpp>

#include <string>


inline void L_StringifyStack(lua_State *L, int count)
{
    int top = lua_gettop(L);

    lua_getglobal(L, "tostring");

    for (int i = 0; i < count; i++)
    {
        if (lua_isstring(L, top - i))
            continue;

        // Call `tostring` to convert the value.
        lua_pushvalue(L, -1);  // the `tostring` function
        lua_pushvalue(L, top - i);
        lua_call(L, 1, 1);

        if (!lua_isstring(L, -1))
        {
            lua_pop(L, 1);
            luaL_error(L, LUA_QL("tostring") " must return a string");
            return;
        }

        // Replace original with converted value.
        lua_replace(L, top - i);
    }

    lua_pop(L, 1);
}


template<PrintFn_t *&PrintFn>
inline int L_Print(lua_State *L)
{
    auto print = *PrintFn;

    int argc = lua_gettop(L);

    L_StringifyStack(L, argc);

    for (int i = 1; i <= argc; i++)
    {
        if (i > 1)
            print("\t");

        print("%s", lua_tostring(L, i));
    }

    print("\n");

    lua_pop(L, argc);
    return 0;
}


inline void L_SetGlobalFunction(lua_State *L, const char *name, lua_CFunction fn)
{
    lua_pushcfunction(L, fn);
    lua_setglobal(L, name);
}


template<typename ...Paths>
inline void L_SetPackagePath(lua_State *L, Paths... paths)
{
    std::string lua_path;
    std::string lua_cpath;

    for (const char *path : { paths... })
    {
        lua_path.append(path);
        lua_path.append("?.lua;");

        lua_path.append(path);
        lua_path.append("?\\init.lua;");

        lua_cpath.append(path);
        lua_cpath.append("?.dll;");

        // This seems to be in `cpath` by default, probably a good idea to keep it.
        lua_cpath.append(path);
        lua_cpath.append("loadall.dll;");
    }

    // Pop trailing semicolon.
    if constexpr (sizeof...(Paths) > 0)
    {
        lua_path.pop_back();
        lua_cpath.pop_back();
    }

    lua_getglobal(L, "package");

    lua_pushstring(L, lua_path.c_str());
    lua_setfield(L, -2, "path");

    lua_pushstring(L, lua_cpath.c_str());
    lua_setfield(L, -2, "cpath");

    lua_pop(L, 1);
}


inline bool L_TryCall(lua_State *L, int argc, int retc)
{
    // Insert error handler above called function.
    int base = lua_gettop(L) - argc;
    lua_pushcfunction(L, [](lua_State *L) {
        L_StringifyStack(L, 1);
        luaL_traceback(L, L, lua_tostring(L, -1), 1);
        return 1;
    });
    lua_insert(L, base);

    // Call the function.
    int status = lua_pcall(L, argc, retc, base);

    // Remove error handler.
    lua_remove(L, base);

    if (status != LUA_OK)
    {
        // Show message returned from error handler.
        Warn("%s\n", lua_tostring(L, -1));
        lua_pop(L, 1);

        return false;
    }

    return true;
}


inline bool L_RunFile(lua_State *L, const char *file_path, int argc, const char *argv[], int retc = 0)
{
    auto top = lua_gettop(L);

    // Load chunk.
    if (luaL_loadfile(L, file_path) != LUA_OK)
    {
        Warn("%s\n", lua_tostring(L, -1));
        lua_pop(L, 1);  // pop error message
        return false;
    }

    // Set global 'arg' table.
    lua_createtable(L, 1 + argc, 0);
    // Script path goes to arg[0].
    lua_pushstring(L, file_path);
    lua_rawseti(L, -2, 0);
    // Arguments follow.
    for (int i = 0; i < argc; ++i)
    {
        lua_pushstring(L, argv[i]);
        lua_rawseti(L, -2, i + 1);
    }
    lua_setglobal(L, "arg");

    // Also pass arguments directly.
    for (int i = 0; i < argc; ++i)
    {
        lua_pushstring(L, argv[i]);
    }

    // Run loaded file.
    return L_TryCall(L, argc, retc);
}


inline bool L_RunFile(lua_State *L, const char *file_path, int retc = 0)
{
    return L_RunFile(L, file_path, 0, nullptr, retc);
}
