# Google Test

if(GTEST_SRC_PATH)
  if(MSVC)
    option(gtest_force_shared_crt "" ON)
  endif(MSVC)
  add_subdirectory(${GTEST_SRC_PATH} "gtest-bin")
  set(GTEST_FOUND true)
  set(GTEST_LIBRARY "gtest")
else(GTEST_SRC_PATH)
  find_package(GTest)
endif(GTEST_SRC_PATH)
