function(extract_component_version
  cmake_parse_arguments(EXTRACT_VERSION
    ""
    "VER_FILE;COMP_PREFIX"
    ""
    ${ARGN}
  )

  if(NOT EXTRACT_COMPONENT_VER_FILE)
    message(FATAL_ERROR "Need a version file to extract version.")
  endif()
)

endfunction(exttace_componen_version
)
