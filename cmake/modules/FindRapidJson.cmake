# =============================================================================
# FILE: FindRapidJson.cmake
# Find RapidJson
#
#==============================================================================

#include(FindPkgConfig)
#find_package(PkgConfig)
#pkg_check_modules(PC_RapidJSON QUIET RapidJSON)

find_path(RapidJSON_INCLUDE_DIR
  NAMES rapidjson.h
  PATHS ${RAPIDJSON_HOME}/include
  PATH_SUFFIXES rapidjson
)
message("RapidJSON_INCLUDE_DIR: ${RapidJSON_INCLUDE_DIR}")

# get version. It is defined in CMakeLists and put into RapidJSONConfig.cmake =>
# extract version from RapidJSONConfigVersion.cmake
file(STRINGS ${RAPIDJSON_HOME}/cmake/RapidJSONConfigVersion.cmake version_line REGEX "PACKAGE_VERSION \"")
string(REGEX MATCH "([0-9]+\.[0-9]+\.[0-9]+)" _ ${version_line})
message("version_line: ${version_line}")
message("CMAKE_MATCH_0: ${CMAKE_MATCH_0}")
set(RapidJSON_VERSION ${CMAKE_MATCH_0})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(RapidJSON
    REQUIRED_VARS RapidJSON_INCLUDE_DIR
    VERSION_VAR RapidJSON_VERSION
)

message("RapidJSON_FOUND: ${RapidJSON_FOUND}")
message("RapidJSON_VERSION: ${RapidJSON_VERSION}")

if(RapidJSON_FOUND AND NOT TARGET RapidJSON::RapidJSON)
    add_library(RapidJSON::RapidJSON INTERFACE IMPORTED)
    set_target_properties(RapidJSON::RapidJSON PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${RAPIDJSON_HOME}/include
    )
endif()

mark_as_advanced(
  RapidJSON_FOUND 
  RapidJSON_INCLUDE_DIR 
  RapidJSON_VERSION
)