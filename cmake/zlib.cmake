# zlib
find_package(ZLIB)

if(ZLIB_FOUND)
  # only to satisfy dependencies
  add_custom_target(zlib)
else(ZLIB_FOUND)
  include(ExternalProject)
  # An external project for zlib
  set(zlib_source  "${CMAKE_CURRENT_BINARY_DIR}/zlib-src")
  set(zlib_build   "${CMAKE_CURRENT_BINARY_DIR}/zlib")
  set(zlib_install "${CMAKE_CURRENT_BINARY_DIR}/prefix")
  set(zlib_file "http://zlib.net/zlib128.zip")
  set(zlib_md5 "126f8676442ffbd97884eb4d6f32afb4")

  if(CMAKE_CFG_INTDIR STREQUAL ".")
    set(zlib_build_type ${CMAKE_BUILD_TYPE})
  else()
    set(zlib_build_type ${CMAKE_CFG_INTDIR})
  endif()

  ExternalProject_Add(zlib
    DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}
    SOURCE_DIR ${zlib_source}
    BINARY_DIR ${zlib_build}
    INSTALL_DIR ${zlib_install}
    URL ${zlib_file}
    URL_MD5 ${zlib_md5}
    PATCH_COMMAND ${CMAKE_COMMAND} -E remove <SOURCE_DIR>/zconf.h
    CMAKE_CACHE_ARGS
    -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
    -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
    -DCMAKE_BUILD_TYPE:STRING=${zlib_build_type}
    ${zlib_EXTRA_ARGS}
    CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>)

  set(ZLIB_INCLUDE_DIRS "${zlib_install}/include/")
  set(ZLIB_INCLUDES "${ZLIB_INCLUDE_DIRS}/zconf.h" "${ZLIB_INCLUDE_DIRS}/zlib.h")

  if(MSVC)
    set(ZLIB_LIBRARIES ${zlib_install}/lib/zlibstatic$<$<CONFIG:Debug>:d>.lib)
  endif(MSVC)
  if(MINGW)
    set(ZLIB_LIBRARIES "${zlib_install}/lib/libzlibstatic.a")
  endif(MINGW)
  if(UNIX)
    set(ZLIB_LIBRARIES "${zlib_install}/lib/libz.a")
  endif(UNIX)
endif(ZLIB_FOUND)

include_directories(${ZLIB_INCLUDE_DIRS})
