if(WIN32)
  set(BASEPYEXE py.exe -3)
else(WIN32)
  set(BASEPYEXE python3)
endif(WIN32)

set(VENV_DIR ${CMAKE_CURRENT_BINARY_DIR}/msgpack-venv)

if(WIN32)
  set(PYEXE ${VENV_DIR}/Scripts/python.exe)
else(WIN32)
  set(PYEXE ${VENV_DIR}/bin/python)
endif(WIN32)

add_custom_command(
  OUTPUT ${PYEXE}
  COMMAND ${BASEPYEXE} -m virtualenv ${VENV_DIR})

if(WIN32)
  set(SIX_LOC ${VENV_DIR}/Lib/site-packages/six.py)
else(WIN32)
  set(SIX_LOC ${VENV_DIR}/lib/site-packages/six.py)
endif(WIN32)

add_custom_command(
  OUTPUT ${SIX_LOC}
  COMMAND ${PYEXE} -m pip install six
  DEPENDS ${PYEXE})

add_custom_target(cpp_python_gen
  DEPENDS ${SIX_LOC})
