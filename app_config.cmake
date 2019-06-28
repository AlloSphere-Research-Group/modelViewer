# allolib location
set(allolib_directory allolib)

# list your app files here
set(app_files_list
  src/main.cpp
)

# other directories to include
set(app_include_dirs
  ${CMAKE_CURRENT_LIST_DIR}/src
)

# other libraries to link
set(app_link_libs
)

# definitions
set(app_definitions
)

# Additional compile flags
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
set(app_compile_flags
    /bigobj /F2000000
)
endif()

# linker flags, with `-` in the beginning
set(app_linker_flags
)
