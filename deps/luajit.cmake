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

  # Extract `libluajit.so` soname from the Makefile.
  execute_process(
    COMMAND make -f getvar.mk TARGET_SONAME -- -C "${LUAJIT_SOURCE_DIR}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    OUTPUT_VARIABLE LUAJIT_SONAME
    COMMAND_ERROR_IS_FATAL ANY
  )
  string(STRIP "${LUAJIT_SONAME}" LUAJIT_SONAME)

  if(LUAJIT_SONAME STREQUAL "")
    message(
      FATAL_ERROR
      "Failed to extract `libluajit.so` soname from the LuaJIT Makefile."
    )
  endif()

  add_custom_command(
    OUTPUT
      "${LUAJIT_SOURCE_DIR}/libluajit.so"
    WORKING_DIRECTORY "${LUAJIT_SOURCE_DIR}"
    COMMAND make BUILDMODE=dynamic amalg
  )

  add_custom_command(
    OUTPUT
      "${LUAJIT_SOURCE_DIR}/${LUAJIT_SONAME}"
    DEPENDS
      "${LUAJIT_SOURCE_DIR}/libluajit.so"
    WORKING_DIRECTORY "${LUAJIT_SOURCE_DIR}"
    COMMAND
      "${CMAKE_COMMAND}" -E copy_if_different
      "libluajit.so"
      "${LUAJIT_SONAME}"
  )

  add_custom_target(
    luajit.target
    DEPENDS
      "${LUAJIT_SOURCE_DIR}/${LUAJIT_SONAME}"
  )

  set_target_properties(
    luajit PROPERTIES
    IMPORTED_LOCATION "${LUAJIT_SOURCE_DIR}/${LUAJIT_SONAME}"
    IMPORTED_SONAME "${LUAJIT_SONAME}"
  )

else()

  message(FATAL_ERROR "Platform not supported")

endif()
