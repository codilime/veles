set(SERVER_DIR ${CMAKE_CURRENT_BINARY_DIR}/veles-server)
set(SERVER_OUTPUT_STARTUP_SCRIPT_FILE ${SERVER_DIR}/srv.py)

add_custom_target(copy-server-files ALL
  COMMAND cmake -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/python ${CMAKE_CURRENT_BINARY_DIR}/python)

add_custom_command(OUTPUT ${SERVER_OUTPUT_STARTUP_SCRIPT_FILE}
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  "${CMAKE_CURRENT_BINARY_DIR}/python/srv.py"
  "${SERVER_OUTPUT_STARTUP_SCRIPT_FILE}"
  COMMENT "Copying server script")

if(WIN32)
  if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
    set(BASEPYEXE py.exe -3)
  else("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
    set(BASEPYEXE py.exe -3.6-32)
  endif("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
  set(SERVER_PYTHON_DIR ${SERVER_DIR}/python)
  set(SERVER_DIR_DESTINATION "/")
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

  add_custom_command(OUTPUT ${SERVER_OUTPUT_REQUIRMENTS_FILE}
    COMMAND ${BASEPYEXE} -m pip install -r ${CMAKE_CURRENT_BINARY_DIR}/python/requirements.txt -t ${SERVER_DIR}/python
    COMMENT "Installing veles python lib requirements")

  add_custom_command(OUTPUT ${SERVER_OUTPUT_EMBED_PYTHON_FILE}
    COMMAND ${CMAKE_COMMAND} -E tar xzf ${EMBED_PYTHON_ARCHIVE_PATH}
    WORKING_DIRECTORY ${SERVER_PYTHON_DIR}
    COMMENT "Installing server embed python")

  add_custom_command(OUTPUT ${SERVER_OUTPUT_VELES_LIB_FILE}
    COMMAND ${BASEPYEXE} setup.py install --install-lib ${SERVER_PYTHON_DIR_NATIVE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/python/
    COMMENT "Installing veles python lib")

  add_custom_target(server
    DEPENDS ${SERVER_OUTPUT_REQUIRMENTS_FILE}
    DEPENDS ${SERVER_OUTPUT_EMBED_PYTHON_FILE}
    DEPENDS ${SERVER_OUTPUT_VELES_LIB_FILE}
    DEPENDS ${SERVER_OUTPUT_STARTUP_SCRIPT_FILE}
  )

endif(WIN32)


if(CMAKE_HOST_UNIX AND NOT CMAKE_HOST_APPLE)
  set(BASEPYEXE python3)
  set(SERVER_DIR_DESTINATION share)
  # create venv with dependencies after installation and remove it when removing package
  set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
      ${CMAKE_CURRENT_SOURCE_DIR}/resources/install/postinst
      ${CMAKE_CURRENT_SOURCE_DIR}/resources/install/prerm)


  set(SERVER_OUTPUT_REQUIRMENTS_FILE ${SERVER_DIR}/requirements.txt)

  add_custom_command(OUTPUT ${SERVER_OUTPUT_REQUIRMENTS_FILE}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_BINARY_DIR}/python/requirements.txt"
    "${SERVER_DIR}/requirements.txt"
    COMMENT "Copying server script")

  set(SERVER_OUTPUT_VELES_LIB_FILE ${SERVER_PYTHON_DIR}/veles)

  add_custom_command(OUTPUT ${SERVER_OUTPUT_VELES_LIB_FILE}
    COMMAND ${BASEPYEXE} setup.py sdist --dist-dir ${SERVER_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/python/
    COMMENT "Installing veles python lib")

  add_custom_target(server
    DEPENDS ${SERVER_OUTPUT_REQUIRMENTS_FILE}
    DEPENDS ${SERVER_OUTPUT_STARTUP_SCRIPT_FILE}
    DEPENDS ${SERVER_OUTPUT_VELES_LIB_FILE}
  )

endif(CMAKE_HOST_UNIX AND NOT CMAKE_HOST_APPLE)

if (CMAKE_HOST_APPLE)
  set(BASEPYEXE python3)
  set(SERVER_DIR_DESTINATION "veles.app/Contents/Resources/")
  set(SERVER_PYTHON_VENV_DIR ${SERVER_DIR}/venv)

  set(SERVER_OUTPUT_VELES_VENV_PYTHON ${SERVER_DIR}/venv/bin/python3)
  add_custom_command(OUTPUT ${SERVER_OUTPUT_VELES_VENV_PYTHON}
    COMMAND ${BASEPYEXE} -m venv ${SERVER_PYTHON_VENV_DIR}
    COMMENT "Creating veles python virtual enviroment")

  set(SERVER_OUTPUT_VELES_LIB_FILE ${SERVER_DIR}/veleslib)
  add_custom_command(OUTPUT ${SERVER_OUTPUT_VELES_LIB_FILE}
    COMMAND ${SERVER_OUTPUT_VELES_VENV_PYTHON} setup.py install
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/python/
    DEPENDS ${SERVER_OUTPUT_VELES_VENV_PYTHON}
    COMMENT "Installing veles python lib")

  set(SERVER_OUTPUT_VELES_LIB_REQUIRMENTS ${SERVER_DIR}/requirements)
  add_custom_command(OUTPUT ${SERVER_OUTPUT_VELES_LIB_REQUIRMENTS}
    COMMAND ${SERVER_OUTPUT_VELES_VENV_PYTHON} -m pip install -r requirements.txt
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/python/
    DEPENDS ${SERVER_OUTPUT_VELES_VENV_PYTHON}
    COMMENT "Installing veles python lib requirements")

  add_custom_target(server
    DEPENDS ${SERVER_OUTPUT_STARTUP_SCRIPT_FILE}
    DEPENDS ${SERVER_OUTPUT_VELES_LIB_FILE}
    DEPENDS ${SERVER_OUTPUT_VELES_LIB_REQUIRMENTS}
  )

endif(CMAKE_HOST_APPLE)


# prepare server environment only in install target
install(CODE "execute_process(COMMAND \"${CMAKE_COMMAND}\" --build \"${CMAKE_CURRENT_BINARY_DIR}\" --target server)")
install(DIRECTORY ${SERVER_DIR} DESTINATION ${SERVER_DIR_DESTINATION} COMPONENT "server")


