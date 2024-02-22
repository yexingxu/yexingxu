cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

project(ipc LANGUAGES CXX)

include(GNUInstallDirs)
include(CheckAndAddFiles)


include_directories(${GTEST_INCLUDE_DIRS})


# check_and_add_files(IPC_SOURCES
#  "src/shm/"
#   shared_memory.cc
#   memory_map.cc
#   utils.cc
#   signal_handle.cc
#   shared_memory_object.cc
#   algorithm.cc
#   file_management_interface.cc
#   user.cc
#   group.cc
#   file_access.cc
#   memory_provider.cc
#   shared_memory_provider.cc
#   memory_manager_master.cc
#   memory_block.cc
#   memory.cc
#   relative_pointer_data.cc
# )

# check_and_add_files(IPC_SOURCES
#  "src/memory/"
#   chunk_setting.cc
#   chunk_header.cc
#   bump_allocator.cc
#   loffli.cc
#   memory_pool.cc
#   memory_manager.cc
#   shared_chunk.cc
#   config.cc
# )

# check_and_add_files(IPC_SOURCES
#  "src/entity/"
#   publisher_port_user.cc
#   base_port.cc
#   shm_safe_unmanaged_chunk.cc
#   condition_notifier.cc
#   mutex.cc
#   locking_policy.cc
# )

# check_and_add_files(IPC_SOURCES
#  "src/sem/"
#   semaphore_interface.cc
#   unnamed_semaphore.cc
# )

check_and_add_files(IPC_SOURCES
 "src/infrastructure/semaphore/"
  unnamed_semaphore.cc
)

check_and_add_files(IPC_SOURCES
 "src/infrastructure/"
  duration.cc
)

add_library(ipc SHARED ${IPC_SOURCES})

set_target_properties(ipc
  PROPERTIES
    POSITION_INDEPENDENT_CODE ON

    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
)

target_compile_options(ipc
  PRIVATE
    -Wall -Wextra -Wshadow -Wswitch-default
    # -Weffc++ -Werror  -pedantic
    $<$<CONFIG:Debug>:-Wsign-conversion -ggdb>
)

target_include_directories(ipc
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ipc
  PRIVATE
    ${GTEST_BOTH_LIBRARIES}
    pthread
    rt
    # Threads::Threades
)

# add_subdirectory(test)

#install
install(TARGETS ipc RUNTIME DESTINATION ipc/bin)
