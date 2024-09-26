local banner = [[
If you're seeing this message in-game, it means that the plugin is working as
intended. It may still fail to load, but that is because the example Lua script
only supports specific versions of engine interfaces.
]]

banner = banner:gsub("\n", " ")
print(banner)


local ffi = require "ffi"


-- Define FFI ctypes for interacting with the engine.

-- Add `thiscall` calling convention to a function pointer.
local function thiscall(ctype)
  if ffi.os == "Windows" then
    return "__thiscall " .. ctype
  else
    return ctype .. " __attribute__((thiscall))"
  end
end

ffi.cdef [[
// These definitions are taken from <tier1/convar.h> in source-sdk-2013.

enum
{
  COMMAND_MAX_ARGC = 64,
  COMMAND_MAX_LENGTH = 512,
};

typedef struct CCommand
{
  int m_nArgc;
  int m_nArgv0Size;
  char m_pArgSBuffer[ COMMAND_MAX_LENGTH ];
  char m_pArgvBuffer[ COMMAND_MAX_LENGTH ];
  const char* m_ppArgv[ COMMAND_MAX_ARGC ];
}
CCommand;

typedef void ( *FnCommandCallback_t )( const CCommand &command );

typedef struct ConCommand
{
  // Virtual function table poiner.
  void **__vfptr;

  // ConCommandBase data members.

  void *m_pNext;

  bool m_bRegistered;

  const char *m_pszName;
  const char *m_pszHelpString;

  int m_nFlags;

  // ConCommand data members.

  FnCommandCallback_t m_fnCommandCallback;
  void *m_fnCompletionCallback;

  bool m_bHasCompletionCallback : 1;
  bool m_bUsingNewCommandCallback : 1;
  bool m_bUsingCommandCallbackInterface : 1;
}
ConCommand;
]]


-- The `ICvar` engine interface.
local icvar = nil


-- Get a virtual function pointer from the virtual table of an object.
local function vfunc(this, index)
  local vtable = ffi.cast("void ***", this)[0]
  return vtable[index]
end


-- Define the plugin and its functions.

local Plugin = {
  command = nil
}


function Plugin:Load(create_interface)
  create_interface = ffi.cast(
    "void * (*)(const char *name, int *returnCode)",
    create_interface
  )

  icvar = create_interface("VEngineCvar004", nil)
  if icvar == nil then
    warn("ICvar interface not found")
    return false
  end

  local ICvar__RegisterConCommand = ffi.cast(
    thiscall("void (*)(void *this, ConCommand *command)"),
    vfunc(icvar, 6)
  )
  local ICvar__FindCommand = ffi.cast(
    thiscall("ConCommand * (*)(void *this, const char *name)"),
    vfunc(icvar, 14)
  )

  -- Find some existing ConCommand and use its vtable pointer for our commands.
  local reference_command = ICvar__FindCommand(icvar, "echo")
  if reference_command == nil then
    warn("Could not find reference command")
    return false
  end

  ConCommand__vtptr = ffi.cast("void ***", reference_command)[0]

  -- Callback function for our command.
  self.callback = ffi.cast("FnCommandCallback_t", function(args)
    args = ffi.cast("CCommand *", args)

    -- Contains the whole command string (including the command itself).
    local argstring = ffi.string(args.m_pArgSBuffer)
    local chunk = argstring:gsub("^.-lua", "")

    -- This is what most Lua interpreters do.
    chunk = chunk:gsub("^%s*=", "return ")

    loaded, errmsg = loadstring(chunk)
    if loaded == nil then
      warn(errmsg)
    else
      print(loaded())
    end
  end)

  self.command = ffi.new("ConCommand", {
    __vfptr = ConCommand__vtptr,
    m_pNext = nil,
    m_bRegistered = false,
    m_pszName = "lua",
    m_pszHelpString = "Run a Lua chunk and print its results",
    m_nFlags = 0,
    m_fnCommandCallback = self.callback,
    m_fnCompletionCallback = nil,
    m_bHasCompletionCallback = false,
    m_bUsingNewCommandCallback = true,
    m_bUsingCommandCallbackInterface = false,
  })

  ICvar__RegisterConCommand(icvar, self.command)

  return true
end


function Plugin:Unload()
  -- `Unload` is called even when `Load` fails, make sure we can handle that.
  if self.command == nil then
    return
  end

  local ICvar__UnregisterConCommand = ffi.cast(
    thiscall("void (*)(void *this, ConCommand *command)"),
    vfunc(icvar, 7)
  )

  ICvar__UnregisterConCommand(icvar, self.command)

  -- Callbacks have to be freed manually (see LuaJIT FFI docs).
  self.callback:free()
end


function Plugin:GetPluginDescription()
  return "lua_plugin example"
end


return Plugin
