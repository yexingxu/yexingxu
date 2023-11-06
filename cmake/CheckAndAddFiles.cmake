#check if the files are exist, and add them to files list.
#if the files are not exist, print error message and exit.

function(check_and_add_files files_list path_prefix)
  set(new_list "${${files_list}}")
  foreach(file ${ARGN})
    get_filename_component(full_file "${path_prefix}${file}" ABSOLUTE)
    if(NOT EXISTS "${full_file}")
      message(FATAL_ERROR "File ${full_file} does not exist.")
    endif()

    list(APPEND new_list "${full_file}")
  endforeach()

  set(${files_list} "${new_list}" PARENT_SCOPE)

endfunction()
