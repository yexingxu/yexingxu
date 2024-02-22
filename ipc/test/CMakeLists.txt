cmake_minimum_required(VERSION 3.11 FATAL_ERROR)


project(ipc-test LANGUAGES CXX)

include(GNUInstallDirs)
include(CheckAndAddFiles)

find_package(GTest REQUIRED)


include_directories(${GTEST_INCLUDE_DIRS})

check_and_add_files(TEST_SOURCES
 "infrastructure/"
    test_unnamed_semaphore.cc
)

add_executable(ipc-test ${TEST_SOURCES})


set_target_properties(ipc-test
  PROPERTIES
    POSITION_INDEPENDENT_CODE ON

    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
)

target_compile_options(ipc-test
  PRIVATE
    -Wall -Wextra -Wshadow -Wswitch-default
    # -Weffc++ -Werror  -pedantic
    $<$<CONFIG:Debug>:-Wsign-conversion -ggdb>
)

target_include_directories(ipc-test
 PRIVATE
  ../include
)

target_link_libraries(ipc-test
  PRIVATE
    ${GTEST_BOTH_LIBRARIES}
    GTest::gtest
    GTest::gmock
    ipc
    rt
)

#install
install(TARGETS ipc-test RUNTIME DESTINATION ipc-test/bin)
