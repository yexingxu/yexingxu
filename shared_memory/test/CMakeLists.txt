cmake_minimum_required(VERSION 3.11 FATAL_ERROR)


project(shm-test LANGUAGES CXX)

include(GNUInstallDirs)
include(CheckAndAddFiles)

# find_package(Threads REQUIRED)
find_package(GTest REQUIRED)
# find_package(shm REQUIRED)


include_directories(${GTEST_INCLUDE_DIRS})

check_and_add_files(TEST_SOURCES
 "./"
  # test_shared_memory.cc
  # test_shared_memory_object.cc
  # test_memory_provider.cc
  # test_shared_memory_provider.cc
  # test_chunk_setting.cc
  # test_chunk_header.cc
  # test_bump_allocator.cc
  # test_memory_pool.cc
  # test_memory_manager.cc
  test_semgent.cc
)

add_executable(shm-test ${TEST_SOURCES})


set_target_properties(shm-test
  PROPERTIES
    POSITION_INDEPENDENT_CODE ON

    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
)

target_compile_options(shm-test
  PRIVATE
    -Wall -Wextra -Wshadow -Wswitch-default
    # -Weffc++ -Werror  -pedantic
    $<$<CONFIG:Debug>:-Wsign-conversion -ggdb>
)

target_include_directories(shm-test
 PRIVATE
  ../include
)

target_link_libraries(shm-test
  PRIVATE
    ${GTEST_BOTH_LIBRARIES}
    GTest::gtest
    GTest::gmock
    shm
    rt
)

#install
install(TARGETS shm-test RUNTIME DESTINATION shm-test/bin)
