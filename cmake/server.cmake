if(WIN32)
  set(BASEPYEXE py.exe -3)
  set(SERVER_DIR ${CMAKE_CURRENT_BINARY_DIR}/veles-server)
  set(SERVER_PYTHON_DIR ${SERVER_DIR}/python)
  file(TO_NATIVE_PATH ${SERVER_PYTHON_DIR} SERVER_PYTHON_DIR_NATIVE)

  if(NOT EMBED_PYTHON_ARCHIVE_PATH)
    set(EMBED_PYTHON_URL "https://www.python.org/ftp/python/3.6.1/python-3.6.1-embed-win32.zip")
    set(EMBED_PYTHON_ARCHIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/python-3.6.1-embed-win32.zip")
    set(EMBED_PYTHON_SHA256 "1b1adada30ccaf8b001d83f3008408df6bfc98beb2c5e42cd341cbe484386a50")
    file(DOWNLOAD ${EMBED_PYTHON_URL} ${EMBED_PYTHON_ARCHIVE_PATH} EXPECTED_HASH SHA256=${EMBED_PYTHON_SHA256})
  endif(NOT EMBED_PYTHON_ARCHIVE_PATH)

  # assume that at least six is present in python/requirements.txt
  set(SERVER_OUTPUT_REQUIRMENTS_FILE ${SERVER_PYTHON_DIR}/six.py)
  set(SERVER_OUTPUT_EMBED_PYTHON_FILE ${SERVER_PYTHON_DIR}/python.exe)
  set(SERVER_OUTPUT_VELES_LIB_FILE ${SERVER_PYTHON_DIR}/veles/__init__.py)
  set(SERVER_OUTPUT_STARTUP_SCRIPT_FILE ${SERVER_DIR}/srv.py)

  add_custom_command(OUTPUT ${SERVER_OUTPUT_REQUIRMENTS_FILE}
    COMMAND ${BASEPYEXE} -m pip install -r ${CMAKE_SOURCE_DIR}/python/requirements.txt -t ${SERVER_DIR}/python
    COMMENT "Installing veles python lib requirements")

  add_custom_command(OUTPUT ${SERVER_OUTPUT_EMBED_PYTHON_FILE}
    COMMAND ${CMAKE_COMMAND} -E tar xzf ${EMBED_PYTHON_ARCHIVE_PATH}
    WORKING_DIRECTORY ${SERVER_PYTHON_DIR}
    COMMENT "Installing server embed python")

  add_custom_command(OUTPUT ${SERVER_OUTPUT_STARTUP_SCRIPT_FILE}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/python/srv.py"
    "${SERVER_OUTPUT_STARTUP_SCRIPT_FILE}"
    COMMENT "Copying server script")

  add_custom_command(OUTPUT ${SERVER_OUTPUT_VELES_LIB_FILE}
    COMMAND ${BASEPYEXE} setup.py install --install-lib ${SERVER_PYTHON_DIR_NATIVE}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/python/
    COMMENT "Installing veles python lib")

  add_custom_target(server
    DEPENDS ${SERVER_OUTPUT_REQUIRMENTS_FILE}
    DEPENDS ${SERVER_OUTPUT_EMBED_PYTHON_FILE}
    DEPENDS ${SERVER_OUTPUT_VELES_LIB_FILE}
    DEPENDS ${SERVER_OUTPUT_STARTUP_SCRIPT_FILE}
  )

  # prepare server environment only in install target
  install(CODE "execute_process(COMMAND \"${CMAKE_COMMAND}\" --build \"${CMAKE_CURRENT_BINARY_DIR}\" --target server)")
  install(DIRECTORY ${SERVER_DIR} DESTINATION "/" COMPONENT "server")

endif(WIN32)
