set(LUAJIT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/luajit/src")

add_library(luajit SHARED IMPORTED GLOBAL)

target_include_directories(
  luajit
  INTERFACE "${LUAJIT_SOURCE_DIR}"
)

add_dependencies(luajit luajit.target)

if(WIN32)

  add_custom_command(
    OUTPUT
      "${LUAJIT_SOURCE_DIR}/lua51.lib"
      "${LUAJIT_SOURCE_DIR}/lua51.dll"
    WORKING_DIRECTORY "${LUAJIT_SOURCE_DIR}"
    # Use `msvcbuild.bat` if building with MSVC and `make` otherwise.
    COMMAND "$<IF:$<BOOL:${MSVC}>,msvcbuild.bat;amalg,make;amalg>"
    COMMAND_EXPAND_LISTS
    VERBATIM
  )

  add_custom_target(
    luajit.target
    DEPENDS
      "${LUAJIT_SOURCE_DIR}/lua51.dll"
      "${LUAJIT_SOURCE_DIR}/lua51.lib"
  )

  set_target_properties(
    luajit PROPERTIES
    IMPORTED_IMPLIB "${LUAJIT_SOURCE_DIR}/lua51.lib"
    IMPORTED_LOCATION "${LUAJIT_SOURCE_DIR}/lua51.dll"
  )

elseif(LINUX)

  add_custom_command(
    OUTPUT
      "${LUAJIT_SOURCE_DIR}/libluajit.so"
    WORKING_DIRECTORY "${LUAJIT_SOURCE_DIR}"
    COMMAND make BUILDMODE=dynamic amalg
  )

  add_custom_target(
    luajit.target
    DEPENDS
      "${LUAJIT_SOURCE_DIR}/libluajit.so"
  )

  set_target_properties(
    luajit PROPERTIES
    IMPORTED_LOCATION "${LUAJIT_SOURCE_DIR}/libluajit.so"
  )

else()

  message(FATAL_ERROR "Platform not supported")

endif()
