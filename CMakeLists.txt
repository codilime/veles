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

project(VELES CXX)
if(WIN32)
  cmake_minimum_required(VERSION 3.7.0)
else(WIN32)
  cmake_minimum_required(VERSION 3.1.0)
endif(WIN32)

# Basic packaging data
set(CPACK_PACKAGE_VENDOR "Codilime")
set(CPACK_PACKAGE_NAME "veles")
set(CPACK_DEBIAN_PACKAGE_SECTION "Editors")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "VELES hex editor")

include("cmake/gtest.cmake")
include("cmake/qt.cmake")
include("cmake/zlib.cmake")
# Compiler flags
set(CMAKE_CXX_STANDARD_REQUIRED 11) # Use c++11
set(CMAKE_INCLUDE_CURRENT_DIR ON)
if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall --std=c++11") # All warnings
endif()
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc ") # set exception handling mode and linking mode
endif()
if(APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif(APPLE)

# Sources
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(TEST_DIR ${CMAKE_SOURCE_DIR}/test)

include_directories(${INCLUDE_DIR})

# VELES build rules
# LIB: veles_base
add_library(veles_base
    ${INCLUDE_DIR}/util/sampling/isampler.h
    ${INCLUDE_DIR}/util/sampling/uniform_sampler.h
    ${INCLUDE_DIR}/util/sampling/fake_sampler.h
    ${INCLUDE_DIR}/util/settings/theme.h
    ${INCLUDE_DIR}/util/settings/hexedit.h
    ${INCLUDE_DIR}/util/encoders/encoder.h
    ${INCLUDE_DIR}/util/encoders/factory.h
    ${INCLUDE_DIR}/util/encoders/base64_encoder.h
    ${INCLUDE_DIR}/util/encoders/hex_encoder.h
    ${SRC_DIR}/util/sampling/isampler.cc
    ${SRC_DIR}/util/sampling/uniform_sampler.cc
    ${SRC_DIR}/util/sampling/fake_sampler.cc
    ${SRC_DIR}/util/settings/theme.cc
    ${SRC_DIR}/util/settings/hexedit.cc
    ${SRC_DIR}/util/encoders/encoder.cc
    ${SRC_DIR}/util/encoders/base64_encoder.cc
    ${SRC_DIR}/util/encoders/hex_encoder.cc
    ${SRC_DIR}/util/encoders/factory.cc
    ${SRC_DIR}/util/version.cc)

qt5_use_modules(veles_base Core Gui Widgets)

# LIB: veles_visualisation

qt5_wrap_ui(VISUALISATION_FORMS
    ${SRC_DIR}/visualisation/selectrangedialog.ui
    )

qt5_add_resources(VISUALISATION_SHADERS ${SRC_DIR}/visualisation/shaders/shaders.qrc)
add_library(veles_visualisation
    ${INCLUDE_DIR}/visualisation/panel.h
    ${INCLUDE_DIR}/visualisation/base.h
    ${INCLUDE_DIR}/visualisation/digram.h
    ${INCLUDE_DIR}/visualisation/trigram.h
    ${INCLUDE_DIR}/visualisation/minimap.h
    ${INCLUDE_DIR}/visualisation/minimap_panel.h
    ${INCLUDE_DIR}/visualisation/selectrangedialog.h
    ${SRC_DIR}/visualisation/panel.cc
    ${SRC_DIR}/visualisation/base.cc
    ${SRC_DIR}/visualisation/digram.cc
    ${SRC_DIR}/visualisation/trigram.cc
    ${SRC_DIR}/visualisation/minimap.cc
    ${SRC_DIR}/visualisation/minimap_panel.cc
    ${SRC_DIR}/visualisation/selectrangedialog.cc
    ${VISUALISATION_FORMS}
    ${VISUALISATION_SHADERS})

qt5_use_modules(veles_visualisation Core Gui Widgets)
target_link_libraries(veles_visualisation veles_base)

# LIB: veles_data
add_library(veles_data
    ${INCLUDE_DIR}/data/types.h
    ${INCLUDE_DIR}/data/bindata.h
    ${INCLUDE_DIR}/data/repack.h
    ${INCLUDE_DIR}/data/field.h
    ${SRC_DIR}/data/bindata.cc
    ${SRC_DIR}/data/repack.cc
)

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

# LIB: parser
add_library(parser
    ${INCLUDE_DIR}/parser/stream.h
    ${INCLUDE_DIR}/parser/unpyc.h
    ${INCLUDE_DIR}/parser/unpng.h
    ${SRC_DIR}/parser/unpyc.cc
    ${SRC_DIR}/parser/unpng.cc
    ${SRC_DIR}/parser/utils.cc
)

qt5_use_modules(parser Core)
add_dependencies(parser zlib)
target_link_libraries(parser veles_db veles_dbif ${ZLIB_LIBRARIES})

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

qt5_use_modules(veles_db Core)

target_link_libraries(veles_db veles_dbif parser)

# EXE: dbif_test
add_executable(dbif_test ${SRC_DIR}/dbif_test.cc)

qt5_use_modules(dbif_test Core)

target_link_libraries(dbif_test veles_db)

# EXE: unpyc
add_executable(unpyc ${SRC_DIR}/unpyc.cc)

qt5_use_modules(unpyc Core)

target_link_libraries(unpyc veles_db parser)

#target_link_libraries(test_veles veles)

# Resources
qt5_add_resources(RESOURCES resources/veles.qrc)
qt5_wrap_ui(FORMS
    ${SRC_DIR}/ui/optionsdialog.ui
    ${SRC_DIR}/ui/searchdialog.ui
    ${SRC_DIR}/ui/gotoaddressdialog.ui
    ${SRC_DIR}/ui/createchunkdialog.ui
    ${SRC_DIR}/ui/databaseinfo.ui
    )

# EXE: Main executable
add_executable(main_ui
    ${INCLUDE_DIR}/ui/veles_mainwindow.h
    ${INCLUDE_DIR}/ui/hexedittab.h
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
    ${SRC_DIR}/ui/main.cc
    ${SRC_DIR}/ui/veles_mainwindow.cc
    ${SRC_DIR}/ui/hexedittab.cc
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
    ${RESOURCES}
    ${VISUALISATION_SHADERS}
    ${FORMS})

  set (GENERATED_MOC_FILES
    ${CMAKE_SOURCE_DIR}/moc_base.cpp
    ${CMAKE_SOURCE_DIR}/moc_commands.cpp
    ${CMAKE_SOURCE_DIR}/moc_digram.cpp
    ${CMAKE_SOURCE_DIR}/moc_veles_mainwindow.cpp
    ${CMAKE_SOURCE_DIR}/moc_hexedittab.cpp
    ${CMAKE_SOURCE_DIR}/moc_optionsdialog.cpp
    ${CMAKE_SOURCE_DIR}/moc_hexedit.cpp
    ${CMAKE_SOURCE_DIR}/moc_searchdialog.cpp
    ${CMAKE_SOURCE_DIR}/moc_trigram.cpp)

set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${GENERATED_MOC_FILES})

qt5_use_modules(main_ui Core Gui Widgets)

target_link_libraries(veles_visualisation veles_base)
if(GTEST_FOUND)
    include_directories(${GTEST_INCLUDE_DIR})
    add_executable(run_test
        ${TEST_DIR}/run_test.cc
        ${TEST_DIR}/data/bindata.cc
        ${TEST_DIR}/data/copybits.cc
        ${TEST_DIR}/data/repack.cc
        ${TEST_DIR}/util/encoders/hex_encoder.cc
        ${TEST_DIR}/util/encoders/base64_encoder.cc
        ${TEST_DIR}/util/encoders/factory.cc
    )

    qt5_use_modules(run_test Core)

    target_link_libraries(run_test veles_db veles_base ${GTEST_LIBRARY})

    add_custom_command(TARGET run_test
      COMMENT "Running tests"
      COMMAND $<TARGET_FILE:run_test> "--gtest_output=xml:results.xml"
      DEPENDS $<TARGET_FILE:run_test>)
else(GTEST_FOUND)

    message("gtest not found - tests won't be built")

endif(GTEST_FOUND)


#target_link_libraries(test_veles veles)
target_link_libraries(main_ui veles_base veles_db veles_visualisation Qt5::Widgets parser)
set_target_properties(main_ui PROPERTIES OUTPUT_NAME "veles")
if(WIN32 AND NOT MSVC)
  set_target_properties(main_ui PROPERTIES RUNTIME_OUTPUT_DIRECTORY "bin")
endif(WIN32 AND NOT MSVC)


# Post-build packaging

#Unix paths
if(CMAKE_HOST_UNIX AND NOT CMAKE_HOST_APPLE)
  install(TARGETS main_ui RUNTIME DESTINATION bin)
endif(CMAKE_HOST_UNIX AND NOT CMAKE_HOST_APPLE)

# Windows
if(WIN32)
  # Run winddeployqt if it can be found
  find_program(WINDEPLOYQT_EXECUTABLE NAMES windeployqt HINTS ${QTDIR} ENV QT PATH_SUFFIXES bin)
  add_custom_command(TARGET main_ui POST_BUILD
    COMMENT "Ensuring QT Dependencies"
    COMMAND ${WINDEPLOYQT_EXECUTABLE} ${WINDEPLOYQT_ARGS} $<TARGET_FILE:main_ui>
    DEPENDS $<TARGET_FILE:main_ui>)
  install(DIRECTORY $<TARGET_FILE_DIR:main_ui> DESTINATION "bin")
  set(CPACK_PACKAGE_EXECUTABLES "veles.exe" "VELES")
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
  install(TARGETS main_ui BUNDLE DESTINATION "veles")
  set(CPACK_PACKAGE_EXECUTABLES "veles.app" "VELES")
endif(APPLE)

# Few common CPack settings
set(CPACK_MONOLITHIC_INSTALL)
set(CPACK_STRIP_FILES false)
INCLUDE(CPack)
