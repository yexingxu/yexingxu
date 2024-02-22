cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

# include(ExtractComponentVersion)
# exteract_component_version(test_ver
#   VER_FILE "${CMAKE_CURRENT_SOURCE_DIR}/include/public/version.h"
#   COMP_PREFIX "TEST"
# )

project(shm LANGUAGES CXX)

include(GNUInstallDirs)
include(CheckAndAddFiles)

# find_package(Threads REQUIRED)
find_package(spdlog REQUIRED)
if(NOT spdlog_FOUND)
  message(STATUS "There is no GTest in sysroot ${CMAKE_SYSROOT}")
  add_subdirectory(../thirdpart/spdlog)
endif()

include_directories(${GTEST_INCLUDE_DIRS})


check_and_add_files(SHM_SOURCES
 "src/shm/"
  shared_memory.cc
  memory_map.cc
  utils.cc
  signal_handle.cc
  shared_memory_object.cc
  algorithm.cc
  file_management_interface.cc
  user.cc
  group.cc
  file_access.cc
  memory_provider.cc
  shared_memory_provider.cc
  memory_manager_master.cc
  memory_block.cc
  memory.cc
  relative_pointer_data.cc
)

check_and_add_files(SHM_SOURCES
 "src/memory/"
  chunk_setting.cc
  chunk_header.cc
  bump_allocator.cc
  loffli.cc
  memory_pool.cc
  memory_manager.cc
  shared_chunk.cc
  config.cc
)

check_and_add_files(SHM_SOURCES
 "src/entity/"
  publisher_port_user.cc
  base_port.cc
  shm_safe_unmanaged_chunk.cc
  condition_notifier.cc
  mutex.cc
  locking_policy.cc
)

check_and_add_files(SHM_SOURCES
 "src/sem/"
  semaphore_interface.cc
  unnamed_semaphore.cc
)

check_and_add_files(SHM_SOURCES
 "src/types/"
  unique_port_id.cc
)

add_library(shm SHARED ${SHM_SOURCES})

set_target_properties(shm
  PROPERTIES
    POSITION_INDEPENDENT_CODE ON

    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
)

target_compile_options(shm
  PRIVATE
    -Wall -Wextra -Wshadow -Wswitch-default
    # -Weffc++ -Werror  -pedantic
    $<$<CONFIG:Debug>:-Wsign-conversion -ggdb>
)

target_include_directories(shm
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(shm
  PRIVATE
    ${GTEST_BOTH_LIBRARIES}
    pthread
    spdlog
    rt
    # Threads::Threades
)

add_subdirectory(test)

#install
install(TARGETS shm RUNTIME DESTINATION shm/bin)
