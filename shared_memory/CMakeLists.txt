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
 "src/details/"
  shared_memory.cc
  memory_map.cc
  utils.cc
  signal_handle.cc
)

check_and_add_files(SHM_SOURCES
 "src/"
  shared_memory_object.cc

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

#install
install(TARGETS shm RUNTIME DESTINATION shm/bin)
