# lua_plugin

Source engine plugin that embeds [LuaJIT][LuaJIT] and delegates the plugin
callbacks to Lua.

This is useful mainly for writing simple client plugins. I wouldn't recommend it
for anything complex, since Lua's debugging capabilities are rather limited
compared to C++. Also there is no SDK...

This README assumes that you are already familiar with writing plugins for
Source engine games and Lua.


## Compatibility

Both 32 and 64-bit games are supported, on both Windows and Linux. Keep it mind
that you need a different version of the plugin binaries for each of those four
combinations.

### Source

All Source engine games _should_ work.

<details>
  <summary>Technical details</summary>

  The plugin does not depend on any engine interfaces and only uses logging
  functions from [`tier0`][tier0]. All it does is expose its callbacks under
  versions 1 to 3 of the [`IServerPluginCallbacks`][IServerPluginCallbacks]
  interface. Any game that supports those versions will work.

</details>


### Source 2

Source 2 has a different plugin system. It is **not supported** and never will
be.


## Quickstart

This section uses Windows filenames. On Linux, substitute `lua_plugin.dll` and
`lua51.dll` for `lua_plugin.so` and `libluajit.*.so.*`, respectively.

To create a new standalone plugin:

1. Download and extract `lua_plugin.dll` and `lua51.dll` from the
   [latest release](https://github.com/jooonior/lua_plugin/releases/latest).

2. Rename `lua_plugin.dll` to `<your plugin name>.dll`.

3. Create `<your plugin name>.lua` (or `<your plugin name>/init.lua`) next to
   the plugin DLL.

   ```lua
   local Plugin = {}

   function Plugin:Load()
      print("Hello World!")
   end

   return Plugin
   ```

   See [`mod/addons/lua_plugin.lua`](mod/addons/lua_plugin.lua) for a more
   complex example.

4. Pray you didn't make any mistakes with FFI and it doesn't segfault.

Note that `lua51.dll` is linked dynamically, which means that the plugin will
not work without it.

## Semantics

When loaded, the plugin binary finds and executes a Lua module matching its
name (that is `?.lua` or `?/init.lua` where `?` is the plugin binary path
without extension). This Lua module is expected to return a table of functions
corresponding to the [`IServerPluginCallbacks`][IServerPluginCallbacks]
callbacks. These functions are optional --- missing ones will be handled in the
plugin binary.

Each plugin callback delegates to a Lua function of the same name (if defined).
Arguments are forwarded to Lua and return values are forwarded back. Pointer and
reference arguments are passed as _light userdata_ and have to be cast using
[`ffi.cast`][ffi.cast] to their respective _ctypes_ before being useful.

The only modifications to the Lua environment are:

- `print` and `warn` print to the in-game console
- `arg[0]` contains the full path to the main Lua module
- [`package.path`][package.path] is set to `?.lua` and `?/init.lua` inside of
  the directory with the plugin binary
- [`package.cpath`][package.cpath] is set to `?.dll`/`?.so` and
  `loadall.dll`/`loadall.so` inside of the directory with the plugin binary
- `INTERFACEVERSION_ISERVERPLUGINCALLBACKS` is set to the selected
  `IServerPluginCallbacks` interface version

No other integration with the engine is implemented. You are expected to use
LuaJIT's [`ffi`][ffi] library for interacting with the engine.


## Changelog

See [CHANGELOG.md](./CHANGELOG.md).


## Building

The plugin and its dependencies are built with CMake. Check out the
[`build` GitHub action](./.github/actions/build/action.yml) to see how.


## Debugging

You can debug the plugin straight from Visual Studio 2022. Debugging is
configured in [`.vs/launch.vs.json`](./.vs/launch.vs.json). Keep in mind that
you can't debug a 32-bit build in a 64-bit game (and vice versa).

[LuaJIT]: https://luajit.org/
[IServerPluginCallbacks]: https://developer.valvesoftware.com/wiki/IServerPluginCallbacks
[tier0]: https://github.com/ValveSoftware/source-sdk-2013/blob/master/mp/src/public/tier0/dbg.h
[ffi]: https://luajit.org/ext_ffi.html
[ffi.cast]: https://luajit.org/ext_ffi_api.html#ffi_cast
[package.path]: https://www.lua.org/manual/5.1/manual.html#pdf-package.path
[package.cpath]: https://www.lua.org/manual/5.1/manual.html#pdf-package.cpath
