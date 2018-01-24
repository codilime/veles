# zlib
find_package(ZLIB)

if(ZLIB_FOUND)
  # Only to satisfy dependencies
  add_custom_target(zlib)
else()
  include(ExternalProject)
  # An external project for zlib
  set(ZLIB_SOURCE  "${CMAKE_CURRENT_BINARY_DIR}/zlib-src")
  set(ZLIB_BUILD   "${CMAKE_CURRENT_BINARY_DIR}/zlib")
  set(ZLIB_INSTALL "${CMAKE_CURRENT_BINARY_DIR}/prefix")
  set(ZLIB_FILE "https://zlib.net/fossils/zlib-1.2.8.tar.gz")
  set(ZLIB_MD5 "44d667c142d7cda120332623eab69f40")

  if(CMAKE_CFG_INTDIR STREQUAL ".")
    set(ZLIB_BUILD_TYPE ${CMAKE_BUILD_TYPE})
  else()
    set(ZLIB_BUILD_TYPE ${CMAKE_CFG_INTDIR})
  endif()

  ExternalProject_Add(
      zlib
      DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}
      SOURCE_DIR ${ZLIB_SOURCE}
      BINARY_DIR ${ZLIB_BUILD}
      INSTALL_DIR ${ZLIB_INSTALL}
      URL ${ZLIB_FILE}
      # `ExternalProject_Add` currently doesn't support anything other than MD5.
      URL_MD5 ${ZLIB_MD5}
      PATCH_COMMAND ${CMAKE_COMMAND} -E remove "<SOURCE_DIR>/zconf.h"
      CMAKE_CACHE_ARGS
      -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
      -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
      -DCMAKE_BUILD_TYPE:STRING=${ZLIB_BUILD_TYPE}
      ${ZLIB_EXTRA_ARGS}
      CMAKE_ARGS
      -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
  )

  set(ZLIB_INCLUDE_DIRS "${ZLIB_INSTALL}/include/")
  set(ZLIB_INCLUDES "${ZLIB_INCLUDE_DIRS}/zconf.h" "${ZLIB_INCLUDE_DIRS}/zlib.h")

  if(MSVC)
    set(ZLIB_LIBRARIES "${ZLIB_INSTALL}/lib/zlibstatic$<$<CONFIG:Debug>:d>.lib")
  endif()
  if(UNIX)
    set(ZLIB_LIBRARIES "${ZLIB_INSTALL}/lib/libz.a")
  endif()
endif()

include_directories(${ZLIB_INCLUDE_DIRS})
