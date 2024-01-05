cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

# include(ExtractComponentVersion)
# exteract_component_version(test_ver
#   VER_FILE "${CMAKE_CURRENT_SOURCE_DIR}/include/public/version.h"
#   COMP_PREFIX "TEST"
# )

project(serialize-test LANGUAGES CXX)

include(GNUInstallDirs)
include(CheckAndAddFiles)

# find_package(Threads REQUIRED)
find_package(GTest REQUIRED)
# if(NOT GTest_FOUND)
#   message(STATUS "There is no GTest in sysroot ${CMAKE_SYSROOT}")
#   add_subdirectory(../thirdpart/googletest)
# endif()

include_directories(${GTEST_INCLUDE_DIRS})

check_and_add_files(TEST_SOURCES
 "test/"
  test_helper.cc
)
check_and_add_files(TEMPLATE_TEST_SOURCES
 "test/"
  type_id_test.cc
  calculate_size_test.cc
  # type_calculate_test.cc
  serializer_test.cc
  # deserializer_test.cc
)
add_executable(serialize-test ${TEST_SOURCES} ${TEMPLATE_TEST_SOURCES})

set_target_properties(serialize-test
  PROPERTIES
    POSITION_INDEPENDENT_CODE ON

    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
)

target_compile_options(serialize-test
  PRIVATE
    -Wall -Wextra -Wshadow -Wswitch-default
    # -Weffc++ -Werror  -pedantic
    $<$<CONFIG:Debug>:-Wsign-conversion -ggdb>
)

target_include_directories(serialize-test
 PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/
)

target_link_libraries(serialize-test
  PRIVATE
    ${GTEST_BOTH_LIBRARIES}
    pthread
)

#install
install(TARGETS serialize-test RUNTIME DESTINATION serialize-test/bin)
