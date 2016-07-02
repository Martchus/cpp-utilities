# before including this module, the project meta-data must be set

# set project name (displayed in Qt Creator)
project(${META_PROJECT_NAME})

# provide variables for other projects built as part of the same subdirs project
# to access files from this project
get_directory_property(HAS_PARENT PARENT_DIRECTORY)
if(HAS_PARENT)
    set(${META_PROJECT_VARNAME}_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" PARENT_SCOPE)
    set(${META_PROJECT_VARNAME}_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}" PARENT_SCOPE)
    set(${META_PROJECT_NAME}_DIR "${CMAKE_CURRENT_BINARY_DIR}" PARENT_SCOPE)
endif()

# stringify the meta data
set(META_PROJECT_NAME_STR "\"${META_PROJECT_NAME}\"")
set(META_APP_VERSION ${META_VERSION_MAJOR}.${META_VERSION_MINOR}.${META_VERSION_PATCH})
set(META_APP_NAME_STR "\"${META_APP_NAME}\"")
set(META_APP_AUTHOR_STR "\"${META_APP_AUTHOR}\"")
set(META_APP_URL_STR "\"${META_APP_URL}\"")
set(META_APP_DESCRIPTION_STR "\"${META_APP_DESCRIPTION}\"")
set(META_APP_VERSION_STR "\"${META_APP_VERSION}\"")

# set META_PROJECT_VARNAME and META_PROJECT_VARNAME_UPPER if not specified explicitely
if(NOT META_PROJECT_VARNAME)
    set(META_PROJECT_VARNAME ${META_PROJECT_NAME})
endif()
if(NOT META_PROJECT_VARNAME_UPPER)
    string(TOUPPER ${META_PROJECT_VARNAME} META_PROJECT_VARNAME_UPPER)
endif()

# set TARGET_EXECUTABLE which is used to refer to the target executable at its installation location
set(TARGET_EXECUTABLE "${CMAKE_INSTALL_PREFIX}/bin/${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}")

# find config.h template
include(TemplateFinder)
find_template_file("config.h" CPP_UTILITIES CONFIG_H_TEMPLATE_FILE)

# add configuration header
configure_file(
    "${CONFIG_H_TEMPLATE_FILE}"
    "${CMAKE_CURRENT_BINARY_DIR}/resources/config.h"
)

# ensure generated include files are found
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# disable new ABI (can't catch ios_base::failure with new ABI)
set(FORCE_OLD_ABI "no" CACHE STRING "specifies whether usage of old ABI should be forced")
if(${FORCE_OLD_ABI} STREQUAL "yes")
    add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
    message(STATUS "Forcing usage of old CXX11 ABI.")
endif()

# enable debug-only code when doing a debug build
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG_BUILD)
    message(STATUS "Debug build enabled.")
endif()
