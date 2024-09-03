# lua_plugin

Source engine plugin that embeds [LuaJIT][LuaJIT] and delegates the plugin
callbacks to Lua.

This is useful mainly for writing simple client plugins. I wouldn't recommend it
for anything complex, since Lua's debugging capabilities are rather limited
compared to C++. Also there is no SDK...

This README assumes that you are already familiar with writing plugins for
Source engine games and Lua.


## Compatibility


### Source

Any Source engine game should work. This plugin supports both 32 and 64-bit
games, but you need to download the correct version for your platform.


### Source 2

Source 2 has a different plugin system. It is **not supported** and never will
be.


## Quickstart

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

When loaded, the plugin DLL finds and executes a Lua module matching its name
(that is `?.lua` or `?/init.lua` where `?` is the DLL path without extension).
This Lua module is expected to return a table of functions corresponding to the
[`IServerPluginCallbacks`][IServerPluginCallbacks] callbacks. These functions
are optional --- missing ones will be handled in the plugin DLL.

Each plugin callback delegates to a Lua function of the same name (if defined).
Arguments are forwarded to Lua and return values are forwarded back. Pointer and
reference arguments are passed as _light userdata_ and have to be cast using
[`ffi.cast`][ffi.cast] to their respective _ctypes_ before being useful.

The only modifications to the Lua environment are:

- `print` and `warn` print to the in-game console
- `arg[0]` contains the full path to the main Lua module
- [`package.path`][package.path] is set to `?.lua` and `?/init.lua` inside of
  the directory with the plugin DLL
- [`package.cpath`][package.cpath] is set to `?.dll` and `loadall.dll` inside of
  the directory with the plugin DLL

No other integration with the engine is implemented. You are expected to use
LuaJIT's [`ffi`][ffi] library for interacting with the engine.


## Changelog

See [CHANGELOG.md](./CHANGELOG.md).


## Building

All dependencies are included as git submodules and integrated in the solution.
Building it with Visual Studio should just work(TM). At least it does for me in
VS2022...


## Debugging

You can debug the plugin straight from Visual Studio. To do so, set these
**Debugging** configuration properties for the **lua_plugin** project:

- Command: Browse for the game executable (`tf_win64.exe`, `tf.exe`, `hl2.exe`)
- Command Arguments: `-insecure -insert_search_path "$(SolutionDir)mod"`
- Working Directory: `$(LocalDebuggerCommand)\..`

[LuaJIT]: https://luajit.org/
[IServerPluginCallbacks]: https://developer.valvesoftware.com/wiki/IServerPluginCallbacks
[ffi]: https://luajit.org/ext_ffi.html
[ffi.cast]: https://luajit.org/ext_ffi_api.html#ffi_cast
[package.path]: https://www.lua.org/manual/5.1/manual.html#pdf-package.path
[package.cpath]: https://www.lua.org/manual/5.1/manual.html#pdf-package.cpath

Then hit <kbd>F5</kbd> to launch and debug TF2 with the plugin loaded.
