#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "SeqMaker::SeqMaker" for configuration "Release"
set_property(TARGET SeqMaker::SeqMaker APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(SeqMaker::SeqMaker PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/libSeqMaker.dll.a"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libSeqMaker.dll"
  )

list(APPEND _cmake_import_check_targets SeqMaker::SeqMaker )
list(APPEND _cmake_import_check_files_for_SeqMaker::SeqMaker "${_IMPORT_PREFIX}/lib/libSeqMaker.dll.a" "${_IMPORT_PREFIX}/lib/libSeqMaker.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
