cmake_minimum_required(VERSION 3.20)

project(lua_plugin)

set(
  SOURCES
  src/engine.cpp
  src/interface.cpp
  src/platform.cpp
  src/plugin.cpp
)

set(
  HEADERS
  src/engine.hpp
  src/interface.hpp
  src/L.hpp
  src/platform.hpp
  src/plugin.hpp
)

add_subdirectory(deps)

add_library(lua_plugin SHARED ${SOURCES} ${HEADERS})

set_property(TARGET lua_plugin PROPERTY CXX_STANDARD 17)
set_property(TARGET lua_plugin PROPERTY CXX_STANDARD_REQUIRED ON)

# On Linux, CMake prepends `lib` to library outputs by default.
set_target_properties(lua_plugin PROPERTIES PREFIX "")

target_link_libraries(
  lua_plugin
  luajit
  whereami
)

if(MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
  message("Configuring MSVC for hot reload")
  target_compile_options(lua_plugin PUBLIC "/ZI")
  target_link_options(lua_plugin PUBLIC "/INCREMENTAL")
endif()

add_custom_command(
  TARGET lua_plugin
  POST_BUILD
  COMMENT "Copying built binaries to mod directory"
  COMMAND
    "${CMAKE_COMMAND}" -E copy_if_different
    "$<TARGET_FILE:lua_plugin>"
    "$<TARGET_FILE:luajit>"
    "${PROJECT_SOURCE_DIR}/mod/addons/"
)

# Make the plugin load the LuaJIT library from its own directory.

# On Linux, this is done by setting its rpath.
set_target_properties(
  lua_plugin PROPERTIES
  BUILD_WITH_INSTALL_RPATH TRUE
  INSTALL_RPATH "$ORIGIN"
)

# On Windows, the same effect can be achieved by using a custom delay load hook.
if (MSVC)
  target_link_options(lua_plugin PRIVATE "/DELAYLOAD:$<PATH:GET_FILENAME,$<TARGET_FILE:luajit>>")
  target_link_libraries(lua_plugin delayimp)
  target_sources(lua_plugin PRIVATE src/delayhook.cpp)
endif()
