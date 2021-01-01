cmake_minimum_required(VERSION 3.3.0 FATAL_ERROR)

# before including this module, all relevant variables must be set just include this module as last one since nothing should
# depend on it

if (NOT TARGET_CONFIG_DONE)
    message(FATAL_ERROR "Before including the ConfigHeader module, the AppTarget/LibraryTarget module must be included.")
endif ()

# find config.h template
include(TemplateFinder)
find_template_file("config.h" CPP_UTILITIES CONFIG_H_TEMPLATE_FILE)

# ensure certain variables are cleared
unset(DEPENCENCY_VERSIONS)
unset(DEPENCENCY_VERSIONS_ARRAY)
unset(LINK_LIBRARIES_LIST)
unset(INTERFACE_LINK_LIBRARIES_LIST)
unset(PROCESSED_DEPENDENCIES)
unset(HAVE_OPENSSL)

# get list of dependencies the main target links to
if (NOT META_HEADER_ONLY_LIB)
    get_target_property(LINK_LIBRARIES_LIST "${META_TARGET_NAME}" LINK_LIBRARIES)
endif ()
get_target_property(INTERFACE_LINK_LIBRARIES_LIST "${META_TARGET_NAME}" INTERFACE_LINK_LIBRARIES)

# make list with link-time dependency versions and display names
foreach (DEPENDENCY IN LISTS LINK_LIBRARIES_LIST INTERFACE_LINK_LIBRARIES_LIST)
    # skip non-targets and already processed dependencies
    if (NOT TARGET "${DEPENDENCY}" OR "${DEPENDENCY}" IN_LIST PROCESSED_DEPENDENCIES)
        continue()
    endif ()

    # find version and display name for target
    unset(DEPENDENCY_VER)
    unset(DEPENDENCY_DISPLAY_NAME)
    if (DEPENDENCY MATCHES "((Static)?Qt[0-9]+)::([A-Za-z0-9]+)")
        # read meta-data of Qt module
        set(DEPENDENCY_MODULE_PREFIX "${CMAKE_MATCH_1}")
        set(DEPENDENCY_MODULE_NAME "${CMAKE_MATCH_3}")
        set(DEPENDENCY_DISPLAY_NAME "Qt ${DEPENDENCY_MODULE_NAME}")
        set(DEPENDENCY_VER "${${DEPENDENCY_MODULE_PREFIX}${DEPENDENCY_MODULE_NAME}_VERSION_STRING}")
        if (NOT DEPENDENCY_VER)
            set(DEPENDENCY_VER "${${DEPENDENCY_MODULE_PREFIX}${DEPENDENCY_MODULE_NAME}_VERSION}")
        endif ()
    elseif (DEPENDENCY STREQUAL ZLIB::ZLIB)
        set(DEPENDENCY_DISPLAY_NAME "zlib")
        set(DEPENDENCY_VER "${ZLIB_VERSION_STRING}")
    elseif (NOT HAVE_OPENSSAL AND (DEPENDENCY STREQUAL OpenSSL::SSL OR DEPENDENCY STREQUAL OpenSSL::Crypto))
        set(DEPENDENCY_DISPLAY_NAME "OpenSSL")
        set(DEPENDENCY_VER "${OPENSSL_VERSION}")
    elseif (${DEPENDENCY}_varname)
        # read meta-data of one of my own libraries
        set(DEPENDENCY_VARNAME "${${DEPENDENCY}_varname}")
        set(DEPENDENCY_DISPLAY_NAME "${DEPENDENCY}")
        if (${DEPENDENCY_VARNAME}_DISPLAY_NAME)
            set(DEPENDENCY_DISPLAY_NAME "${${DEPENDENCY_VARNAME}_DISPLAY_NAME}")
        endif ()
        set(DEPENDENCY_VER "${${DEPENDENCY_VARNAME}_VERSION}")
    endif ()

    # TODO: provide meta-data for other libs, too

    if (NOT DEPENDENCY_VER OR "${DEPENDENCY_VER}" STREQUAL "DEPENDENCY_VER-NOTFOUND")
        continue()
    endif ()
    list(APPEND PROCESSED_DEPENDENCIES "${DEPENDENCY}")
    list(APPEND DEPENCENCY_VERSIONS "${DEPENDENCY_DISPLAY_NAME}: ${DEPENDENCY_VER}")
endforeach ()

# format "DEPENCENCY_VERSIONS_ARRAY"
if (DEPENCENCY_VERSIONS)
    include(ListToString)
    list_to_string("," " \\\n    \"" "\"" "${DEPENCENCY_VERSIONS}" DEPENCENCY_VERSIONS_ARRAY)
endif ()

# add configuration header
configure_file("${CONFIG_H_TEMPLATE_FILE}" "${CMAKE_CURRENT_BINARY_DIR}/resources/config.h")

# ensure generated include files can be included via #include "resources/config.h"
if (NOT META_HEADER_ONLY_LIB)
    foreach (TARGET_NAME ${META_TARGET_NAME} ${META_TARGET_NAME}_tests ${META_TARGET_NAME}_testlib)
        if (TARGET ${TARGET_NAME})
            target_include_directories(${TARGET_NAME} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
        endif ()
    endforeach ()
endif ()
