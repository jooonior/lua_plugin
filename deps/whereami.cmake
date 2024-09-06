set(WHEREAMI_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/whereami/src")

add_library(
  whereami
  STATIC
  "${WHEREAMI_SOURCE_DIR}/whereami.c"
  "${WHEREAMI_SOURCE_DIR}/whereami.h"
)

target_include_directories(
  whereami
  PUBLIC "${WHEREAMI_SOURCE_DIR}"
)
