if(WIN32)
  set(BASEPYEXE py.exe -3)
else(WIN32)
  set(BASEPYEXE python3)
endif(WIN32)

set(VENV_DIR ${CMAKE_CURRENT_BINARY_DIR}/msgpack-venv)

add_custom_command(
  OUTPUT ${VENV_DIR}
  COMMAND ${BASEPYEXE} -m virtualenv ${VENV_DIR})

if(WIN32)
  set(SIX_LOC ${VENV_DIR}/Lib/site-packages/six.py)
else(WIN32)
  set(SIX_LOC ${VENV_DIR}/lib/site-packages/six.py)
endif(WIN32)

if(WIN32)
  set(PYEXE ${VENV_DIR}/Scripts/python.exe)
else(WIN32)
  set(PYEXE ${VENV_DIR}/bin/python)
endif(WIN32)

add_custom_command(
  OUTPUT ${SIX_LOC}
  COMMAND ${PYEXE} -m pip install six
  DEPENDS ${VENV_DIR})

if(NOT MSGPACK_INCLUDE_PATH)
  set(MSGPACK_URL "https://github.com/msgpack/msgpack-c/releases/download/cpp-2.1.1/msgpack-2.1.1.tar.gz")
  set(MSGPACK_PATH "${CMAKE_CURRENT_BINARY_DIR}/msgpack-2.1.1.tar.gz")
  set(MSGPACK_EXTRACT_PATH "${CMAKE_CURRENT_BINARY_DIR}/msgpack-2.1.1")
  set(MSGPACK_SHA256 "fce702408f0d228a1b9dcab69590d6a94d3938f694b95c9e5e6249617e98d83f")

  file(DOWNLOAD ${MSGPACK_URL} ${MSGPACK_PATH} EXPECTED_HASH SHA256=${MSGPACK_SHA256})

  add_custom_command(
      OUTPUT ${MSGPACK_EXTRACT_PATH}
      COMMAND ${CMAKE_COMMAND} -E tar xzf ${MSGPACK_PATH})
  set(MSGPACK_INCLUDE_PATH "${MSGPACK_EXTRACT_PATH}/include")
  add_custom_target(msgpack-c DEPENDS ${MSGPACK_EXTRACT_PATH} ${SIX_LOC})
else(NOT MSGPACK_INCLUDE_PATH)
  add_custom_target(msgpack-c ${SIX_LOC})
endif(NOT MSGPACK_INCLUDE_PATH)

include_directories(${MSGPACK_INCLUDE_PATH})
