#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "armadillo" for configuration ""
set_property(TARGET armadillo APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(armadillo PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libarmadillo.a"
  )

list(APPEND _cmake_import_check_targets armadillo )
list(APPEND _cmake_import_check_files_for_armadillo "${_IMPORT_PREFIX}/lib/libarmadillo.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
