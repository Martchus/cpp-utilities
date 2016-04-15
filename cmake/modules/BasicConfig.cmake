# before including this module, the project meta-data must be set

# set project name (displayed in Qt Creator)
project(${META_PROJECT_NAME})

# might be useful so other projects built as part of the same subdirs project
# can access files from this project
set(${META_PROJECT_VARNAME}_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" PARENT_SCOPE)
set(${META_PROJECT_VARNAME}_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}" PARENT_SCOPE)
set(${META_PROJECT_NAME}_DIR "${CMAKE_CURRENT_BINARY_DIR}" PARENT_SCOPE)

# stringify the meta data
set(META_PROJECT_NAME_STR "\"${META_PROJECT_NAME}\"")
set(META_APP_VERSION ${META_VERSION_MAJOR}.${META_VERSION_MINOR}.${META_VERSION_PATCH})
set(META_APP_NAME_STR "\"${META_APP_NAME}\"")
set(META_APP_AUTHOR_STR "\"${META_APP_AUTHOR}\"")
set(META_APP_URL_STR "\"${META_APP_URL}\"")
set(META_APP_DESCRIPTION_STR "\"${META_APP_DESCRIPTION}\"")
set(META_APP_VERSION_STR "\"${META_APP_VERSION}\"")

# find config.h template
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/config.h.in")
    # check own source directory
    set(CONFIG_H_TEMPLATE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/config.h.in")
    message(STATUS "Using template for config.h from own source directory.")
elseif(EXISTS "${CPP_UTILITIES_SOURCE_DIR}/cmake/templates/config.h.in")
    # check sources of c++utilities
    set(CONFIG_H_TEMPLATE_FILE "${CPP_UTILITIES_SOURCE_DIR}/cmake/templates/config.h.in")
    message(STATUS "Using template for config.h from c++utilities source directory.")
elseif(EXISTS "${CMAKE_INSTALL_PREFIX}/share/c++utilities/cmake/templates/config.h.in")
    # check installed version of c++utilities
    set(CONFIG_H_TEMPLATE_FILE "${CMAKE_INSTALL_PREFIX}/share/c++utilities/cmake/templates/config.h.in")
    message(STATUS "Using template for config.h from c++utilities installation.")
else()
    message(FATAL_ERROR "Template for config.h file can not be located.")
endif()

# add configuration header
configure_file(
    "${CONFIG_H_TEMPLATE_FILE}"
    "${CMAKE_CURRENT_BINARY_DIR}/resources/config.h"
)

# ensure generated include files are found
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# disable new ABI (can't catch ios_base::failure with new ABI)
add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
message(STATUS "Forcing usage of old CXX11 ABI to be able to catch std::ios_base::failure.")

# enable debug-only code when doing a debug build
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG_BUILD)
    message(STATUS "Debug build enabled.")
endif()
