# - Try to find json-c
# https://cmake.org/Wiki/CMake:How_To_Find_Libraries
# Once done this will define
#  JSONC_FOUND - System has json-c
#  JSONC_INCLUDE_DIRS - The json-c include directories
#  JSONC_LIBRARIES - The libraries needed to use json-c
#  JSONC_DEFINITIONS - Compiler switches required for using json-c

find_package(PkgConfig)
pkg_check_modules(PC_JSONC QUIET json-c)
set(JSONC_DEFINITIONS ${PC_JSONC_CFLAGS_OTHER})

find_path(JSONC_INCLUDE_DIR json.h
          HINTS ${PC_JSONC_INCLUDEDIR} ${PC_JSONC_INCLUDE_DIRS}
          PATH_SUFFIXES json-c)

find_library(JSONC_LIBRARY NAMES json-c libjson-c
             HINTS ${PC_JSONC_LIBDIR} ${PC_JSONC_LIBRARY_DIRS})

find_library(JSONC_LIBRARY NAMES json-c libjson-c
             HINTS ${PC_JSON-C_LIBDIR} ${PC_JSON-C_LIBRARY_DIRS})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set JSONC_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(JSONC DEFAULT_MSG
                                  JSONC_LIBRARY JSONC_INCLUDE_DIR)

if (NOT JSONC_FOUND)

  message(FATAL_ERROR "Json-C is Required!\n")

endif (NOT JSONC_FOUND)

mark_as_advanced(JSONC_INCLUDE_DIR JSONC_LIBRARY)

set(JSONC_LIBRARIES    ${JSONC_LIBRARY})
set(JSONC_INCLUDE_DIRS ${JSONC_INCLUDE_DIR})
