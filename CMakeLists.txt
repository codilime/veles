set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# HACK: ugly hack to provide default compilation type
IF(DEFINED CMAKE_BUILD_TYPE)
   SET(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Choose the type of
build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug
Release RelWithDebInfo MinSizeRel.")
ELSE()
   SET(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build,
options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release
RelWithDebInfo MinSizeRel.")
ENDIF()

project(VELES C CXX)
if(WIN32)
  cmake_minimum_required(VERSION 3.7.0)
else(WIN32)
  cmake_minimum_required(VERSION 3.1.0)
endif(WIN32)

include("cmake/googletest.cmake")
include("cmake/qt.cmake")
include("cmake/zlib.cmake")
include("cmake/msgpack.cmake")
include("cmake/cppgen.cmake")
include("cmake/server.cmake")
# Compiler flags
set(CMAKE_CXX_STANDARD_REQUIRED 11) # Use c++11
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)
set(CMAKE_INSTALL_RPATH ".")
if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall --std=c++11") # All warnings
  set(ADDITIONAL_LINK_LIBRARIES "pthread")
endif()
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc ") # set exception handling mode and linking mode
endif()
if(APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif(APPLE)

if(WIN32)
  set(WINDOWS_APPLICATION_OUT_DIRECTORY "Veles UI")
  if(MSVC)
    # overwrite default Debug/Release application directory
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${WINDOWS_APPLICATION_OUT_DIRECTORY})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${WINDOWS_APPLICATION_OUT_DIRECTORY})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${WINDOWS_APPLICATION_OUT_DIRECTORY})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${WINDOWS_APPLICATION_OUT_DIRECTORY})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${WINDOWS_APPLICATION_OUT_DIRECTORY})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${WINDOWS_APPLICATION_OUT_DIRECTORY})
  endif(MSVC)
endif(WIN32)

# Sources
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(TEST_DIR ${CMAKE_SOURCE_DIR}/test)

include_directories(${INCLUDE_DIR})

# VELES build rules
# LIB: veles_base
add_library(veles_base
    ${INCLUDE_DIR}/util/icons.h
    ${INCLUDE_DIR}/util/concurrency/threadpool.h
    ${INCLUDE_DIR}/util/sampling/isampler.h
    ${INCLUDE_DIR}/util/sampling/uniform_sampler.h
    ${INCLUDE_DIR}/util/sampling/fake_sampler.h
    ${INCLUDE_DIR}/util/settings/theme.h
    ${INCLUDE_DIR}/util/settings/hexedit.h
    ${INCLUDE_DIR}/util/settings/connection_client.h

    ${INCLUDE_DIR}/util/encoders/base64_encoder.h
    ${INCLUDE_DIR}/util/encoders/c_data_encoder.h
    ${INCLUDE_DIR}/util/encoders/c_string_encoder.h
    ${INCLUDE_DIR}/util/encoders/factory.h
    ${INCLUDE_DIR}/util/encoders/hex_encoder.h
    ${INCLUDE_DIR}/util/encoders/idecoder.h
    ${INCLUDE_DIR}/util/encoders/iencoder.h
    ${INCLUDE_DIR}/util/encoders/text_encoder.h
    ${INCLUDE_DIR}/util/encoders/url_encoder.h

    ${INCLUDE_DIR}/util/string_utils.h
    ${INCLUDE_DIR}/util/math.h

    ${SRC_DIR}/util/icons.cc
    ${SRC_DIR}/util/concurrency/threadpool.cc
    ${SRC_DIR}/util/sampling/isampler.cc
    ${SRC_DIR}/util/sampling/uniform_sampler.cc
    ${SRC_DIR}/util/sampling/fake_sampler.cc
    ${SRC_DIR}/util/settings/theme.cc
    ${SRC_DIR}/util/settings/hexedit.cc
    ${SRC_DIR}/util/settings/connection_client.cc

    ${SRC_DIR}/util/encoders/base64_encoder.cc
    ${SRC_DIR}/util/encoders/c_data_encoder.cc
    ${SRC_DIR}/util/encoders/c_string_encoder.cc
    ${SRC_DIR}/util/encoders/factory.cc
    ${SRC_DIR}/util/encoders/hex_encoder.cc
    ${SRC_DIR}/util/encoders/text_encoder.cc
    ${SRC_DIR}/util/encoders/url_encoder.cc

    ${SRC_DIR}/util/string_utils.cc
    ${SRC_DIR}/util/math.cc
    ${SRC_DIR}/util/version.cc)

qt5_use_modules(veles_base Core Gui Widgets)
target_link_libraries(veles_base ${ADDITIONAL_LINK_LIBRARIES})

# LIB: veles_visualization

qt5_wrap_ui(VISUALIZATION_FORMS
    ${SRC_DIR}/visualization/selectrangedialog.ui
    ${SRC_DIR}/visualization/samplingmethoddialog.ui
    )

qt5_add_resources(VISUALIZATION_SHADERS ${SRC_DIR}/visualization/shaders/shaders.qrc)
add_library(veles_visualization
    ${INCLUDE_DIR}/visualization/panel.h
    ${INCLUDE_DIR}/visualization/base.h
    ${INCLUDE_DIR}/visualization/digram.h
    ${INCLUDE_DIR}/visualization/trigram.h
    ${INCLUDE_DIR}/visualization/minimap.h
    ${INCLUDE_DIR}/visualization/minimap_panel.h
    ${INCLUDE_DIR}/visualization/selectrangedialog.h
    ${INCLUDE_DIR}/visualization/manipulator.h
    ${INCLUDE_DIR}/visualization/samplingmethoddialog.h
    ${SRC_DIR}/visualization/panel.cc
    ${SRC_DIR}/visualization/base.cc
    ${SRC_DIR}/visualization/digram.cc
    ${SRC_DIR}/visualization/trigram.cc
    ${SRC_DIR}/visualization/minimap.cc
    ${SRC_DIR}/visualization/minimap_panel.cc
    ${SRC_DIR}/visualization/selectrangedialog.cc
    ${SRC_DIR}/visualization/manipulator.cc
    ${SRC_DIR}/visualization/samplingmethoddialog.cc
    ${VISUALIZATION_FORMS}
    ${VISUALIZATION_SHADERS})

qt5_use_modules(veles_visualization Core Gui Widgets)
target_link_libraries(veles_visualization veles_base)

# LIB: veles_data
add_library(veles_data
    ${INCLUDE_DIR}/data/types.h
    ${INCLUDE_DIR}/data/bindata.h
    ${INCLUDE_DIR}/data/repack.h
    ${INCLUDE_DIR}/data/field.h
    ${INCLUDE_DIR}/data/nodeid.h
    ${SRC_DIR}/data/bindata.cc
    ${SRC_DIR}/data/repack.cc
    ${SRC_DIR}/data/nodeid.cc
)

add_dependencies(veles_data msgpack-c)
qt5_use_modules(veles_data Core)

# LIB: veles_dbif
add_library(veles_dbif
    ${INCLUDE_DIR}/dbif/error.h
    ${INCLUDE_DIR}/dbif/info.h
    ${INCLUDE_DIR}/dbif/method.h
    ${INCLUDE_DIR}/dbif/promise.h
    ${INCLUDE_DIR}/dbif/types.h
    ${INCLUDE_DIR}/dbif/universe.h
    ${SRC_DIR}/dbif/dbif.cc
)

qt5_use_modules(veles_dbif Core)
target_link_libraries(veles_dbif veles_data)

file(GLOB kaitai_headers
    "${INCLUDE_DIR}/kaitai/*.h"
)

file(GLOB kaitai_sources
    "${SRC_DIR}/kaitai/*.cc"
)

# LIB: parser
add_library(parser
    ${INCLUDE_DIR}/parser/parser.h
    ${INCLUDE_DIR}/parser/stream.h
    ${INCLUDE_DIR}/parser/unpyc.h
    ${INCLUDE_DIR}/parser/unpng.h
    ${INCLUDE_DIR}/parser/utils.h
    ${kaitai_headers}
    ${SRC_DIR}/parser/parser.cc
    ${SRC_DIR}/parser/unpyc.cc
    ${SRC_DIR}/parser/unpng.cc
    ${SRC_DIR}/parser/utils.cc
    ${kaitai_sources}
)

qt5_use_modules(parser Core)
add_dependencies(parser zlib)
target_link_libraries(parser veles_db veles_dbif ${ZLIB_LIBRARIES})

set(MSGPACK_CPP "${CMAKE_CURRENT_BINARY_DIR}/messages.h")
set(PYTHON_DIR ${CMAKE_SOURCE_DIR}/python)

add_custom_command(OUTPUT ${MSGPACK_CPP}
  COMMAND ${PYEXE} -m veles.cpp.generate > ${MSGPACK_CPP}
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/python
  COMMENT "Generating msgpack C++ code from Python"
  DEPENDS cpp_python_gen
    ${PYTHON_DIR}/veles/cpp/generate.py
    ${PYTHON_DIR}/veles/proto/node.py
    ${PYTHON_DIR}/veles/proto/messages.py
    ${PYTHON_DIR}/veles/proto/exceptions.py
    ${PYTHON_DIR}/veles/proto/chunk.py
    ${PYTHON_DIR}/veles/schema/model.py
    ${PYTHON_DIR}/veles/schema/enumeration.py
    ${PYTHON_DIR}/veles/schema/fields.py
    ${PYTHON_DIR}/veles/tests/schema/cpp_test_models.py
  VERBATIM)

add_library(veles_network
    ${MSGPACK_CPP}
    ${INCLUDE_DIR}/proto/exceptions.h
    ${INCLUDE_DIR}/network/msgpackwrapper.h
    ${INCLUDE_DIR}/network/msgpackobject.h
    ${SRC_DIR}/network/msgpackobject.cc
    ${SRC_DIR}/network/msgpackwrapper.cc
)

add_dependencies(veles_network msgpack-c)
qt5_use_modules(veles_network Core Network Widgets)


# LIB: veles_db
add_library(veles_db
    ${INCLUDE_DIR}/db/db.h
    ${INCLUDE_DIR}/db/getter.h
    ${INCLUDE_DIR}/db/handle.h
    ${INCLUDE_DIR}/db/object.h
    ${INCLUDE_DIR}/db/types.h
    ${INCLUDE_DIR}/db/universe.h
    ${SRC_DIR}/db/universe.cc
    ${SRC_DIR}/db/object.cc
    ${SRC_DIR}/db/handle.cc
)

# LIB: veles_client
add_library(veles_client
#    ${CMAKE_BINARY_DIR}/messages.h
    ${INCLUDE_DIR}/client/node.h
    ${INCLUDE_DIR}/client/nodetree.h
    ${INCLUDE_DIR}/client/networkclient.h
    ${SRC_DIR}/client/node.cc
    ${SRC_DIR}/client/nodetree.cc
    ${SRC_DIR}/client/networkclient.cc)

qt5_use_modules(veles_client Core Network)
target_link_libraries(veles_client veles_base veles_data veles_network)

qt5_use_modules(veles_db Core)
add_dependencies(veles_db veles_network)
target_link_libraries(veles_db veles_dbif parser)

# EXE: dbif_test
add_executable(dbif_test ${SRC_DIR}/dbif_test.cc)

qt5_use_modules(dbif_test Core)

target_link_libraries(dbif_test veles_db veles_network)

# EXE: unpyc
add_executable(unpyc ${SRC_DIR}/unpyc.cc)

qt5_use_modules(unpyc Core)

target_link_libraries(unpyc veles_db parser veles_network)

#target_link_libraries(test_veles veles)

# Resources
qt5_add_resources(RESOURCES resources/veles.qrc)
qt5_wrap_ui(FORMS
    ${SRC_DIR}/ui/optionsdialog.ui
    ${SRC_DIR}/ui/searchdialog.ui
    ${SRC_DIR}/ui/gotoaddressdialog.ui
    ${SRC_DIR}/ui/createchunkdialog.ui
    ${SRC_DIR}/ui/databaseinfo.ui
    ${SRC_DIR}/ui/logwidget.ui
    ${SRC_DIR}/ui/connectiondialog.ui
    ${SRC_DIR}/ui/connectionnotificationwidget.ui
    )

if(WIN32)
  set(ICONS ${CMAKE_SOURCE_DIR}/resources/icons/veles.rc)
endif(WIN32)

if(APPLE)
  set(ICONS ${CMAKE_SOURCE_DIR}/resources/icons/veles.icns)
  set(MACOSX_BUNDLE_ICON_FILE veles.icns)
  set_source_files_properties(${ICONS} PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources)
endif(APPLE)

if(WIN32)
  # Don't create console window.
  set(GUI_TYPE WIN32)
endif(WIN32)

# EXE: Main executable
add_executable(main_ui
    ${GUI_TYPE}
    ${INCLUDE_DIR}/ui/logwidget.h
    ${INCLUDE_DIR}/ui/veles_mainwindow.h
    ${INCLUDE_DIR}/ui/connectionmanager.h
    ${INCLUDE_DIR}/ui/dockwidget.h
    ${INCLUDE_DIR}/ui/dockwidget_native.h
    ${INCLUDE_DIR}/ui/hexeditwidget.h
    ${INCLUDE_DIR}/ui/nodetreewidget.h
    ${INCLUDE_DIR}/ui/nodewidget.h
    ${INCLUDE_DIR}/ui/optionsdialog.h
    ${INCLUDE_DIR}/ui/hexedit.h
    ${INCLUDE_DIR}/ui/searchdialog.h
    ${INCLUDE_DIR}/ui/gotoaddressdialog.h
    ${INCLUDE_DIR}/ui/slice.h
    ${INCLUDE_DIR}/ui/fileblobitem.h
    ${INCLUDE_DIR}/ui/rootfileblobitem.h
    ${INCLUDE_DIR}/ui/subchunkfileblobitem.h
    ${INCLUDE_DIR}/ui/simplefileblobitem.h
    ${INCLUDE_DIR}/ui/fileblobmodel.h
    ${INCLUDE_DIR}/ui/createchunkdialog.h
    ${INCLUDE_DIR}/ui/databaseinfo.h
    ${INCLUDE_DIR}/ui/spinbox.h
    ${INCLUDE_DIR}/ui/spinboxvalidator.h
    ${INCLUDE_DIR}/ui/connectiondialog.h
    ${SRC_DIR}/ui/main.cc
    ${SRC_DIR}/ui/logwidget.cc
    ${SRC_DIR}/ui/veles_mainwindow.cc
    ${SRC_DIR}/ui/connectionmanager.cc
    ${SRC_DIR}/ui/dockwidget.cc
    ${SRC_DIR}/ui/dockwidget_native.cc
    ${SRC_DIR}/ui/hexeditwidget.cc
    ${SRC_DIR}/ui/nodetreewidget.cc
    ${SRC_DIR}/ui/nodewidget.cc
    ${SRC_DIR}/ui/optionsdialog.cc
    ${SRC_DIR}/ui/hexedit.cc
    ${SRC_DIR}/ui/searchdialog.cc
    ${SRC_DIR}/ui/gotoaddressdialog.cc
    ${SRC_DIR}/ui/fileblobitem.cc
    ${SRC_DIR}/ui/subchunkfileblobitem.cc
    ${SRC_DIR}/ui/rootfileblobitem.cc
    ${SRC_DIR}/ui/fileblobmodel.cc
    ${SRC_DIR}/ui/createchunkdialog.cc
    ${SRC_DIR}/ui/databaseinfo.cc
    ${SRC_DIR}/ui/spinbox.cc
    ${SRC_DIR}/ui/spinboxvalidator.cc
    ${SRC_DIR}/ui/connectiondialog.cc
    ${RESOURCES}
    ${VISUALIZATION_SHADERS}
    ${FORMS}
    ${ICONS})

  set (GENERATED_MOC_FILES
    ${CMAKE_SOURCE_DIR}/moc_base.cpp
    ${CMAKE_SOURCE_DIR}/moc_commands.cpp
    ${CMAKE_SOURCE_DIR}/moc_digram.cpp
    ${CMAKE_SOURCE_DIR}/moc_veles_mainwindow.cpp
    ${CMAKE_SOURCE_DIR}/moc_connectionmanager.cpp
    ${CMAKE_SOURCE_DIR}/moc_dockwidget.cpp
    ${CMAKE_SOURCE_DIR}/moc_hexeditwidget.cpp
    ${CMAKE_SOURCE_DIR}/moc_nodetreewidget.cpp
    ${CMAKE_SOURCE_DIR}/moc_nodewidget.cpp
    ${CMAKE_SOURCE_DIR}/moc_logwidget.cpp
    ${CMAKE_SOURCE_DIR}/moc_optionsdialog.cpp
    ${CMAKE_SOURCE_DIR}/moc_hexedit.cpp
    ${CMAKE_SOURCE_DIR}/moc_searchdialog.cpp
    ${CMAKE_SOURCE_DIR}/moc_trigram.cpp
    ${CMAKE_SOURCE_DIR}/moc_manipulator.cpp
    ${CMAKE_SOURCE_DIR}/moc_connectiondialog.cpp
    ${CMAKE_SOURCE_DIR}/moc_networkclient.cpp)

set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${GENERATED_MOC_FILES})

qt5_use_modules(main_ui Core Gui Widgets)

target_link_libraries(veles_visualization veles_base)
if(GTEST_FOUND AND GMOCK_FOUND)
    include_directories(${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS})
    add_executable(run_test
        ${TEST_DIR}/run_test.cc
        ${TEST_DIR}/data/bindata.cc
        ${TEST_DIR}/data/copybits.cc
        ${TEST_DIR}/data/nodeid.cc
        ${TEST_DIR}/data/repack.cc
        ${TEST_DIR}/network/msgpackobject.cc
        ${TEST_DIR}/network/model.cc
        ${TEST_DIR}/util/encoders/base64_encoder.cc
        ${TEST_DIR}/util/encoders/c_data_encoder.cc
        ${TEST_DIR}/util/encoders/c_string_encoder.cc
        ${TEST_DIR}/util/encoders/hex_encoder.cc
        ${TEST_DIR}/util/encoders/text_encoder.cc
        ${TEST_DIR}/util/encoders/url_encoder.cc
        ${TEST_DIR}/util/encoders/factory.cc
        ${TEST_DIR}/util/sampling/mock_sampler.h
        ${TEST_DIR}/util/sampling/isampler.cc
        ${TEST_DIR}/util/sampling/uniform_sampler.cc
    )

    qt5_use_modules(run_test Core)

    target_link_libraries(run_test veles_db veles_base veles_network ${GTEST_LIBRARIES} ${GMOCK_LIBRARIES})

    add_custom_command(TARGET run_test
      COMMENT "Running tests"
      COMMAND $<TARGET_FILE:run_test> "--gtest_output=xml:results.xml"
      DEPENDS $<TARGET_FILE:run_test>)
else(GTEST_FOUND AND GMOCK_FOUND)

    message("gtest and/or gmock not found - tests won't be built")

endif(GTEST_FOUND AND GMOCK_FOUND)


#target_link_libraries(test_veles veles)
target_link_libraries(main_ui veles_base veles_db veles_network veles_client veles_visualization Qt5::Widgets parser)
set_target_properties(main_ui PROPERTIES OUTPUT_NAME "veles")
if(WIN32 AND NOT MSVC)
  set_target_properties(main_ui PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${WINDOWS_APPLICATION_OUT_DIRECTORY})
endif(WIN32 AND NOT MSVC)

# Post-build packaging

#Unix paths
if(CMAKE_HOST_UNIX AND NOT CMAKE_HOST_APPLE)
  set(CPACK_PACKAGE_CONTACT "contact@codisec.com")
  set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt5core5a (>= 5.5.1), libqt5gui5 (>= 5.5.1), libqt5widgets5 (>= 5.5.1), libqt5network5 (>= 5.5.1), zlib1g (>= 1:1.2.8), python3 (>= 3.5.0), python3-venv (>= 3.5.0)")
  install(TARGETS main_ui RUNTIME DESTINATION bin COMPONENT "application")
endif(CMAKE_HOST_UNIX AND NOT CMAKE_HOST_APPLE)

# Windows
if(WIN32)
  set_target_properties(main_ui PROPERTIES WIN32 TRUE)
  # Run winddeployqt if it can be found
  find_program(WINDEPLOYQT_EXECUTABLE NAMES windeployqt HINTS ${QTDIR} ENV QT PATH_SUFFIXES bin)
  add_custom_command(TARGET main_ui POST_BUILD
    COMMENT "Ensuring QT Dependencies"
    COMMAND ${WINDEPLOYQT_EXECUTABLE} ${WINDEPLOYQT_ARGS} $<TARGET_FILE:main_ui>
    DEPENDS $<TARGET_FILE:main_ui>)
  install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/windeploydir/" DESTINATION ${WINDOWS_APPLICATION_OUT_DIRECTORY} COMPONENT "application")
  install(TARGETS main_ui RUNTIME DESTINATION ${WINDOWS_APPLICATION_OUT_DIRECTORY} COMPONENT "application")
  if(NOT MSVC)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/prefix/bin/libzlib.dll" DESTINATION ${WINDOWS_APPLICATION_OUT_DIRECTORY} COMPONENT "application")
    add_custom_command(TARGET zlib POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${CMAKE_CURRENT_BINARY_DIR}/prefix/bin/libzlib.dll"
      "${CMAKE_CURRENT_BINARY_DIR}/${WINDOWS_APPLICATION_OUT_DIRECTORY}"
      COMMENT "Copying zlib dll")
  endif(NOT MSVC)
  set(CPACK_NSIS_EXECUTABLES_DIRECTORY ${WINDOWS_APPLICATION_OUT_DIRECTORY})
  set(CPACK_NSIS_MUI_FINISHPAGE_RUN "veles")
  set(CPACK_NSIS_INSTALLED_ICON_NAME "${WINDOWS_APPLICATION_OUT_DIRECTORY}/veles.exe")
  set(CPACK_NSIS_DISPLAY_NAME "Veles")
  set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "Codisec")
  set(CPACK_PACKAGE_INSTALL_DIRECTORY "Veles")
  set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
  set(CPACK_NSIS_URL_INFO_ABOUT "https://veles.io")
  set(CPACK_NSIS_CONTACT "contact@codisec.com")
  set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/resources/install/license.txt")
  set(CPACK_PACKAGE_EXECUTABLES "veles" "VELES")
  set(CPACK_CREATE_DESKTOP_LINKS "veles")
  set(CPACK_NSIS_MODIFY_PATH ON)
endif(WIN32)

# Apple
if(APPLE)
  set(CPACK_INSTALL_PREFIX "/Applications")
  # Finding MacDeployQt
  find_program(MACDEPLOYQT_EXECUTABLE
    macdeployqt
    ENV QT
    PATH_SUFFIXES "5.7/clang_64/bin")
  add_custom_command(TARGET main_ui POST_BUILD
    COMMENT "Ensuring QT dependencies"
    COMMAND ${MACDEPLOYQT_EXECUTABLE} ARGS ${CMAKE_CURRENT_BINARY_DIR}/\${CONFIGURATION}/$<TARGET_PROPERTY:main_ui,OUTPUT_NAME>.app ${MACDEPLOYQT_ARGS}
    DEPENDS $<TARGET_FILE:main_ui>)
  set_target_properties(main_ui PROPERTIES MACOSX_BUNDLE TRUE)
  install(TARGETS main_ui BUNDLE DESTINATION "./" COMPONENT "application")
  set(CPACK_PACKAGE_EXECUTABLES "veles.app" "VELES")
endif(APPLE)

set(CPACK_COMPONENT_APPLICATION_DISPLAY_NAME "Veles UI application")
set(CPACK_COMPONENT_SERVER_DISPLAY_NAME "Veles server")
set(CPACK_COMPONENT_APPLICATION_REQUIRED ON)

# Few common CPack settings
set(CPACK_PACKAGE_VENDOR "Codilime")
set(CPACK_PACKAGE_NAME "veles")
string(TIMESTAMP VERSION "%Y.%m")
set(CPACK_PACKAGE_VERSION "${VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "VELES tool for binary data analysis")
set(CPACK_MONOLITHIC_INSTALL)
set(CPACK_STRIP_FILES false)
INCLUDE(CPack)
