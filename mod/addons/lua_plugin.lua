local ffi = require "ffi"


local Plugin = {}


function Plugin:Load(create_interface)
  create_interface = ffi.cast("void * (*)(const char *name, int *returnCode)", create_interface)

  local engine = create_interface("VEngineClient014", nil)
  if engine == nil then
    warn("Engine interface not found")
    return false
  end

  local vtable = ffi.cast("void ***", engine)[0]
  local IVEngineClient__ClientCmd_Unrestricted = ffi.cast(
    "void (*)(void *this, const char *command)",
    vtable[106]
  )

  print("Enabling hacker mode!!!")
  IVEngineClient__ClientCmd_Unrestricted(engine, "sv_cheats 1")

  return true
end


function Plugin:Unload()
  -- stub
end


function Plugin:GetPluginDescription()
  return "lua_plugin example"
end


return Plugin
