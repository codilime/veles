include(FindProtobuf)
find_package(Protobuf)

if(PROTOBUF_FOUND AND (NOT ${PROTOBUF_PROTOC_EXECUTABLE} STREQUAL "PROTOBUF_PROTOC_EXECUTABLE-NOTFOUND") AND (NOT WIN32 OR DEFINED PROTOBUF_DLL_DIR))
  # only to satisfy dependencies
  add_custom_target(protobuf)
  include_directories(${PROTOBUF_INCLUDE_DIRS})
else(PROTOBUF_FOUND AND (NOT ${PROTOBUF_PROTOC_EXECUTABLE} STREQUAL "PROTOBUF_PROTOC_EXECUTABLE-NOTFOUND") AND (NOT WIN32 OR DEFINED PROTOBUF_DLL_DIR))
  include(ExternalProject)
  # An external project for protobuf
  set(protobuf_source  "${CMAKE_CURRENT_BINARY_DIR}/protobuf-src")
  set(protobuf_build   "${CMAKE_CURRENT_BINARY_DIR}/protobuf")
  set(protobuf_install "${CMAKE_CURRENT_BINARY_DIR}/prefix")
  set(protobuf_repo    "https://github.com/google/protobuf.git")
  set(protobuf_version "v3.1.0")

  if(CMAKE_CFG_INTDIR STREQUAL ".")
    set(protobuf_build_type ${CMAKE_BUILD_TYPE})
  else()
    set(protobuf_build_type ${CMAKE_CFG_INTDIR})
  endif()

  # Protobuf CMakeLists.txt isn't in root repository directory and ExternalProject
  # can't handle it properly no matter what options I use so I split it to clone
  # and build steps... (fix for it is added in cmake 3.7)
  ExternalProject_Add(clone_only_protobuf
    SOURCE_DIR ${protobuf_source}
    BINARY_DIR ${protobuf_build}
    INSTALL_DIR ${protobuf_install}
    GIT_REPOSITORY ${protobuf_repo}
    GIT_TAG ${protobuf_version}
    CMAKE_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND "")

  set(protobuf_source  "${CMAKE_CURRENT_BINARY_DIR}/protobuf-src/cmake")

  ExternalProject_Add(protobuf
    DEPENDS clone_only_protobuf
    DOWNLOAD_COMMAND ""
    SOURCE_DIR ${protobuf_source}
    BINARY_DIR ${protobuf_build}
    INSTALL_DIR ${protobuf_install}
    CMAKE_CACHE_ARGS
    -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
    -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
    -DCMAKE_BUILD_TYPE:STRING=${protobuf_build_type}
    -Dprotobuf_BUILD_TESTS:BOOL=OFF
    -Dprotobuf_BUILD_SHARED_LIBS:BOOL=ON
    ${protobuf_EXTRA_ARGS}
    CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    -DCMAKE_INSTALL_RPATH_USE_LINK_PATH:BOOL=ON
    -DCMAKE_INSTALL_RPATH=${protobuf_install}/lib/)

  set(Protobuf_INCLUDE_DIRS "${protobuf_install}/include/")

  if(WIN32)
    set(PROTOBUF_DLL_DIR "${protobuf_install}/bin")
    set(Protobuf_PROTOC_EXECUTABLE "${protobuf_install}/bin/protoc.exe")
    if(MSVC)
      set(PROTOBUF_LIBRARY_DEBUG $<$<CONFIG:Debug>:${protobuf_install}/lib/libprotobufd.lib>)
      set(PROTOBUF_LIBRARY $<$<NOT:$<CONFIG:Debug>>:${protobuf_install}/lib/libprotobuf.lib>)
    endif(MSVC)
    if(MINGW)
      set(PROTOBUF_LIBRARY_DEBUG $<$<CONFIG:Debug>:${protobuf_install}/lib/libprotobufd.dll.a>)
      set(PROTOBUF_LIBRARY $<$<NOT:$<CONFIG:Debug>>:${protobuf_install}/lib/libprotobuf.dll.a>)
    endif(MINGW)
  endif(WIN32)
  if(UNIX)
    # Someone decided that changing variables name is fun and made it backward
    # compatible in a way that doesn't always work so I need to check which one is the correct one.
    # On Windows we require cmake 3.7 so we don't need it
    if(${CMAKE_VERSION} VERSION_GREATER "3.5.2")
      set(Protobuf_PROTOC_EXECUTABLE "${protobuf_install}/bin/protoc")
    else(${CMAKE_VERSION} VERSION_GREATER "3.5.2")
      set(PROTOBUF_PROTOC_EXECUTABLE "${protobuf_install}/bin/protoc")
    endif(${CMAKE_VERSION} VERSION_GREATER "3.5.2")
    if(APPLE)
      set(PROTOBUF_LIBRARIES "${protobuf_install}/lib/libprotobuf$<$<CONFIG:Debug>:d>.dylib")
      # copy protobuf library so that we can use it in veles binary
      add_custom_command(TARGET protobuf POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${protobuf_install}/lib/libprotobuf$<$<CONFIG:Debug>:d>.dylib"
        "${protobuf_install}/lib/libprotoc$<$<CONFIG:Debug>:d>.dylib"
        ${CMAKE_CURRENT_BINARY_DIR})
    else(APPLE)
      set(PROTOBUF_LIBRARIES "${protobuf_install}/lib/libprotobuf$<$<CONFIG:Debug>:d>.so")
    endif(APPLE)
  endif(UNIX)
endif(PROTOBUF_FOUND AND (NOT ${PROTOBUF_PROTOC_EXECUTABLE} STREQUAL "PROTOBUF_PROTOC_EXECUTABLE-NOTFOUND") AND (NOT WIN32 OR DEFINED PROTOBUF_DLL_DIR))

# Needed to use protobuf
add_definitions(-DPROTOBUF_USE_DLLS)

include_directories(${Protobuf_INCLUDE_DIRS})
