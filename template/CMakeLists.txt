cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

project(template LANGUAGES CXX)

include(GNUInstallDirs)


add_executable(template template_test.cc)

set_target_properties(template
  PROPERTIES
    POSITION_INDEPENDENT_CODE ON

    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
)

target_compile_options(template
  PRIVATE
    -Wall -Wextra -pedantic -Wshadow -Wswitch-default
    # -Weffc++ -Werror
    $<$<CONFIG:Debug>:-Wsign-conversion -ggdb>
)

#install
install(TARGETS template RUNTIME DESTINATION template/bin)