cmake_minimum_required(VERSION 3.3.0 FATAL_ERROR)

# prevent multiple inclusion
if(DEFINED THIRD_PARTY_MODULE_LOADED)
    return()
endif()
set(THIRD_PARTY_MODULE_LOADED YES)

macro(save_default_library_suffixes)
    set(DEFAULT_CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})
endmacro()

macro(restore_default_library_suffixes)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ${DEFAULT_CMAKE_FIND_LIBRARY_SUFFIXES})
    unset(DEFAULT_CMAKE_FIND_LIBRARY_SUFFIXES)
endmacro()

macro(configure_static_library_suffixes)
    # allows to look for static libraries in particular
    if(WIN32)
        set(CMAKE_FIND_LIBRARY_SUFFIXES .a .lib)
    else()
        set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
    endif()
endmacro()

macro(configure_dynamic_library_suffixes)
    # allows to look for dynamic libraries in particular
    if(WIN32)
        set(CMAKE_FIND_LIBRARY_SUFFIXES .dll .dll.a)
    elseif(APPLE)
        set(CMAKE_FIND_LIBRARY_SUFFIXES .dylib .so)
    else()
        set(CMAKE_FIND_LIBRARY_SUFFIXES .so)
    endif()
endmacro()

macro(link_against_library_varnames NAME LINKAGE REQUIRED PRIVATE_LIBRARIES_VARNAME PUBLIC_LIBRARIES_VARNAME PRIVATE_STATIC_LIBRARIES_VARNAME PUBLIC_STATIC_LIBRARIES_VARNAME)
    # determine whether the library is required or optional
    # FIXME: improve passing required argument
    if("${REQUIRED}" STREQUAL "OPTIONAL")
        set(${NAME}_REQUIRED "NO")
    elseif("${REQUIRED}" STREQUAL "REQUIRED")
        set(${NAME}_REQUIRED "REQUIRED")
    else()
        message(FATAL_ERROR "Invalid use of link_against_library; must specify either REQUIRED or OPTIONAL.")
    endif()

    # add library to list of libraries to link against when building dynamic libraries or applications
    # - prefer dynamic lib if linkage not explicitely specified
    if(${NAME}_STATIC_LIB AND (("${LINKAGE}" STREQUAL "AUTO_LINKAGE" AND ((NOT (${NAME}_DYNAMIC_LIB OR ${NAME}_SHARED_LIB)) OR (STATIC_LINKAGE AND "${META_PROJECT_TYPE}" STREQUAL "application") OR (STATIC_LIBRARY_LINKAGE AND ("${META_PROJECT_TYPE}" STREQUAL "" OR "${META_PROJECT_TYPE}" STREQUAL "library")))) OR ("${LINKAGE}" STREQUAL "STATIC")))
        set(USE_${NAME} ON)
        set(USE_STATIC_${NAME} ON)
        list(APPEND LIBRARIES ${${NAME}_STATIC_LIB})
        message(STATUS "Linking ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX} statically against external library ${NAME} (${${NAME}_STATIC_LIB}).")
        if(${NAME}_STATIC_INCLUDE_DIR)
            list(APPEND ADDITIONAL_STATIC_INCLUDE_DIRS ${${NAME}_STATIC_INCLUDE_DIR})
            message(STATUS "Adding include path for ${NAME} to ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}: ${${NAME}_STATIC_INCLUDE_DIR}")
        endif()

        if(${${NAME}_STATIC_LIB} IN_LIST META_PUBLIC_STATIC_LIB_DEPENDS OR ${NAME} IN_LIST META_PUBLIC_STATIC_LIB_DEPENDS)
            list(APPEND ${PUBLIC_LIBRARIES_VARNAME} ${${NAME}_STATIC_LIB})
            if(${NAME}_STATIC_INCLUDE_DIR)
                list(APPEND PUBLIC_SHARED_INCLUDE_DIRS ${${NAME}_STATIC_INCLUDE_DIR})
            endif()
        else()
            list(APPEND ${PRIVATE_LIBRARIES_VARNAME} ${${NAME}_STATIC_LIB})
            if(${NAME}_STATIC_INCLUDE_DIR)
                list(APPEND PRIVATE_SHARED_INCLUDE_DIRS ${${NAME}_STATIC_INCLUDE_DIR})
            endif()
        endif()

        # add Qt resources of static library to be enabled
        if(${NAME}_QT_RESOURCES)
            message(STATUS "Adding ${${NAME}_QT_RESOURCES} to LIBRARIES_QT_RESOURCES for ${META_PROJECT_NAME}.")
            list(APPEND LIBRARIES_QT_RESOURCES ${${NAME}_QT_RESOURCES})
        endif()

    elseif((${NAME}_DYNAMIC_LIB OR ${NAME}_SHARED_LIB) AND (("${LINKAGE}" STREQUAL "AUTO_LINKAGE") OR ("${LINKAGE}" STREQUAL "SHARED")))
        set(USE_${NAME} ON)
        set(USE_SHARED_${NAME} ON)
        if(NOT ${NAME}_DYNAMIC_LIB)
            set(${NAME}_DYNAMIC_LIB ${${NAME}_SHARED_LIB})
        endif()
        list(APPEND LIBRARIES ${${NAME}_DYNAMIC_LIB})
        message(STATUS "Linking ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX} dynamically against external library ${NAME} (${${NAME}_DYNAMIC_LIB}).")
        if(${NAME}_DYNAMIC_INCLUDE_DIR)
            list(APPEND ADDITIONAL_SHARED_INCLUDE_DIRS ${${NAME}_DYNAMIC_INCLUDE_DIR})
            message(STATUS "Adding include path for ${NAME} to ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}: ${${NAME}_DYNAMIC_INCLUDE_DIR}")
        endif()

        if(${${NAME}_DYNAMIC_LIB} IN_LIST META_PUBLIC_SHARED_LIB_DEPENDS OR ${NAME} IN_LIST META_PUBLIC_SHARED_LIB_DEPENDS)
            list(APPEND ${PUBLIC_LIBRARIES_VARNAME} ${${NAME}_DYNAMIC_LIB})
            if(${NAME}_DYNAMIC_INCLUDE_DIR)
                list(APPEND PUBLIC_SHARED_INCLUDE_DIRS ${${NAME}_DYNAMIC_INCLUDE_DIR})
            endif()
        else()
            list(APPEND ${PRIVATE_LIBRARIES_VARNAME} ${${NAME}_DYNAMIC_LIB})
            if(${NAME}_DYNAMIC_INCLUDE_DIR)
                list(APPEND PRIVATE_SHARED_INCLUDE_DIRS ${${NAME}_DYNAMIC_INCLUDE_DIR})
            endif()
        endif()
    else()
        if(${NAME}_REQUIRED)
            message(FATAL_ERROR "External library ${NAME} required by ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX} is not available for the specified linkage ${LINKAGE}.")
        else()
            message(WARNING "External library ${NAME} required by ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX} is not available for the specified linkage ${LINKAGE}.")
        endif()
    endif()

    # add library to list of libraries to be provided as transitive dependencies when building static libraries
    # - prefer static lib if linkage not explicitely specified
    if(${NAME}_STATIC_LIB AND ("${LINKAGE}" STREQUAL "AUTO_LINKAGE") OR ("${LINKAGE}" STREQUAL "STATIC"))
        set(USE_${NAME} ON)
        set(USE_STATIC_${NAME} ON)
        list(APPEND STATIC_LIBRARIES ${${NAME}_STATIC_LIB})
        message(STATUS "Adding static external library ${NAME} (${${NAME}_STATIC_LIB}) to dependencies of ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}.")
        if(${NAME}_STATIC_INCLUDE_DIR)
            list(APPEND ADDITIONAL_STATIC_INCLUDE_DIRS ${${NAME}_STATIC_INCLUDE_DIR})
            message(STATUS "Adding include path for ${NAME} to static ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}: ${${NAME}_STATIC_INCLUDE_DIR}")
        endif()

        if(${${NAME}_STATIC_LIB} IN_LIST META_PUBLIC_STATIC_LIB_DEPENDS OR ${NAME} IN_LIST META_PUBLIC_STATIC_LIB_DEPENDS)
            list(APPEND ${PUBLIC_STATIC_LIBRARIES_VARNAME} ${${NAME}_STATIC_LIB})
            if(${NAME}_STATIC_INCLUDE_DIR)
                list(APPEND PUBLIC_STATIC_INCLUDE_DIRS ${${NAME}_STATIC_INCLUDE_DIR})
            endif()
        else()
            list(APPEND ${PRIVATE_STATIC_LIBRARIES_VARNAME} ${${NAME}_STATIC_LIB})
            if(${NAME}_STATIC_INCLUDE_DIR)
                list(APPEND PRIVATE_STATIC_INCLUDE_DIRS ${${NAME}_STATIC_INCLUDE_DIR})
            endif()
        endif()

        # add Qt resources of static library for exporting it
        if(${NAME}_QT_RESOURCES)
            message(STATUS "Adding ${${NAME}_QT_RESOURCES} to STATIC_LIBRARIES_QT_RESOURCES for ${META_PROJECT_NAME}.")
            list(APPEND STATIC_LIBRARIES_QT_RESOURCES ${${NAME}_QT_RESOURCES})
        endif()

    elseif((${NAME}_DYNAMIC_LIB OR ${NAME}_SHARED_LIB) AND (("${LINKAGE}" STREQUAL "AUTO_LINKAGE" AND (NOT ${NAME}_STATIC_LIB OR (NOT STATIC_LINKAGE AND "${META_PROJECT_TYPE}" STREQUAL "application") OR (NOT STATIC_LIBRARY_LINKAGE AND ("${META_PROJECT_TYPE}" STREQUAL "" OR "${META_PROJECT_TYPE}" STREQUAL "library")))) OR ("${LINKAGE}" STREQUAL "SHARED")))
        set(USE_${NAME} ON)
        set(USE_SHARED_${NAME} ON)
        if(NOT ${NAME}_DYNAMIC_LIB)
            set(${NAME}_DYNAMIC_LIB ${${NAME}_SHARED_LIB})
        endif()
        list(APPEND STATIC_LIBRARIES ${${NAME}_DYNAMIC_LIB})
        message(STATUS "Adding dynamic external library ${NAME} (${${NAME}_DYNAMIC_LIB}) to dependencies of static ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}.")
        if(${NAME}_DYNAMIC_INCLUDE_DIR)
            list(APPEND ADDITIONAL_SHARED_INCLUDE_DIRS ${${NAME}_DYNAMIC_INCLUDE_DIR})
            message(STATUS "Adding include path for ${NAME} to static ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}: ${${NAME}_DYNAMIC_INCLUDE_DIR}")
        endif()

        if(${${NAME}_DYNAMIC_LIB} IN_LIST META_PUBLIC_SHARED_LIB_DEPENDS OR ${NAME} IN_LIST META_PUBLIC_SHARED_LIB_DEPENDS)
            list(APPEND ${PUBLIC_STATIC_LIBRARIES_VARNAME} ${${NAME}_DYNAMIC_LIB})
            if(${NAME}_DYNAMIC_INCLUDE_DIR)
                list(APPEND PUBLIC_STATIC_INCLUDE_DIRS ${${NAME}_DYNAMIC_INCLUDE_DIR})
            endif()
        else()
            list(APPEND ${PRIVATE_STATIC_LIBRARIES_VARNAME} ${${NAME}_DYNAMIC_LIB})
            if(${NAME}_DYNAMIC_INCLUDE_DIR)
                list(APPEND PRIVATE_STATIC_INCLUDE_DIRS ${${NAME}_DYNAMIC_INCLUDE_DIR})
            endif()
        endif()
    else()
        if(${NAME}_REQUIRED)
            message(FATAL_ERROR "External library ${NAME} required by ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX} is not available for the specified linkage ${LINKAGE}.")
        else()
            message(WARNING "External library ${NAME} required by ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX} is not available for the specified linkage ${LINKAGE}.")
        endif()
    endif()
endmacro()

macro(link_against_library NAME LINKAGE REQUIRED)
    link_against_library_varnames("${NAME}" "${LINKAGE}" "${REQUIRED}"
        PRIVATE_LIBRARIES PUBLIC_LIBRARIES PRIVATE_STATIC_LIBRARIES PUBLIC_STATIC_LIBRARIES
    )
endmacro()

macro(link_tests_against_library NAME LINKAGE REQUIRED)
    link_against_library_varnames("${NAME}" "${LINKAGE}" "${REQUIRED}"
        TEST_LIBRARIES TEST_LIBRARIES STATIC_TEST_LIBRARIES STATIC_TEST_LIBRARIES
    )
endmacro()

macro(find_external_library NAME LINKAGE REQUIRED)
    set(${NAME}_DYNAMIC_INCLUDE_DIR NOTFOUND CACHE PATH "${NAME} include dir (dynamic)")
    set(${NAME}_DYNAMIC_LIB NOTFOUND CACHE FILEPATH "${NAME} lib (dynamic)")
    set(${NAME}_STATIC_INCLUDE_DIR NOTFOUND CACHE PATH "${NAME} include dir (static)")
    set(${NAME}_STATIC_LIB NOTFOUND CACHE FILEPATH "${NAME} lib (static)")

    save_default_library_suffixes()

    if(NOT ${NAME}_DYNAMIC_LIB)
        configure_dynamic_library_suffixes()
        find_library(DETECTED_${NAME}_DYNAMIC_LIB ${NAME})
        set(${NAME}_DYNAMIC_LIB ${DETECTED_${NAME}_DYNAMIC_LIB} CACHE FILEPATH "${NAME} lib (dynamic)" FORCE)
    endif()

    if(NOT ${NAME}_STATIC_LIB)
        configure_static_library_suffixes()
        find_library(DETECTED_${NAME}_STATIC_LIB ${NAME})
        set(${NAME}_STATIC_LIB ${DETECTED_${NAME}_STATIC_LIB} CACHE FILEPATH "${NAME} lib (static)" FORCE)
    endif()

    restore_default_library_suffixes()
endmacro()

macro(use_external_library NAME LINKAGE REQUIRED)
    find_external_library("${NAME}" "${LINKAGE}" "${REQUIRED}")
    link_against_library("${NAME}" "${LINKAGE}" "${REQUIRED}")
endmacro()

function(use_external_library_from_package_dynamic NAME PKGNAME INCLUDE_VAR LIBRARY_VAR COMPAT_VERSION)
    # internally used by find_external_library_from_package to find dynamic libraries
    configure_dynamic_library_suffixes()
    find_package(${PKGNAME} ${COMPAT_VERSION})
    set(${NAME}_DYNAMIC_INCLUDE_DIR ${${INCLUDE_VAR}} CACHE PATH "${NAME} include dir (dynamic)" FORCE)
    set(${NAME}_DYNAMIC_LIB ${${LIBRARY_VAR}} CACHE FILEPATH "${NAME} lib (dynamic)" FORCE)
endfunction()

function(use_external_library_from_package_static NAME PKGNAME INCLUDE_VAR LIBRARY_VAR COMPAT_VERSION)
    # internally used by find_external_library_from_package to find static libraries
    configure_static_library_suffixes()
    find_package(${PKGNAME} ${COMPAT_VERSION})
    set(${NAME}_STATIC_INCLUDE_DIR ${${INCLUDE_VAR}} CACHE PATH "${NAME} include dir (static)" FORCE)
    set(${NAME}_STATIC_LIB ${${LIBRARY_VAR}} CACHE FILEPATH "${NAME} lib (static)" FORCE)
endfunction()

macro(find_external_library_from_package NAME PKGNAME VERSION INCLUDE_VAR LIBRARY_VAR LINKAGE REQUIRED)
    # handle specified VERSION
    if("${VERSION}" STREQUAL "ANY_VERSION")
        set(${NAME}_COMPATIBLE_VERSION "")
    else()
        set(${NAME}_COMPATIBLE_VERSION ${VERSION})
    endif()

    # use the find_library approach first because it is less buggy when trying to detect static libraries
    # caveat: this way include dirs are not detected - however those are mostly the the default anyways and
    # can also be set manually by the user in case the auto-detection is not sufficient
    find_external_library("${NAME}" "${LINKAGE}" OPTIONAL)

    # fall back to actual use of find_package
    # use separate functions to get a new scope
    save_default_library_suffixes()
    if(NOT ${NAME}_DYNAMIC_LIB)
        use_external_library_from_package_dynamic(${NAME} ${PKGNAME} ${INCLUDE_VAR} "${LIBRARY_VAR}" "${${NAME}_COMPATIBLE_VERSION}")
    endif()
    if(NOT ${NAME}_STATIC_LIB)
        use_external_library_from_package_static(${NAME} ${PKGNAME} ${INCLUDE_VAR} "${LIBRARY_VAR}" "${${NAME}_COMPATIBLE_VERSION}")
    endif()
    restore_default_library_suffixes()
endmacro()

macro(use_external_library_from_package NAME PKGNAME VERSION INCLUDE_VAR LIBRARY_VAR LINKAGE REQUIRED)
    find_external_library_from_package("${NAME}" "${PKGNAME}" "${VERSION}" "${INCLUDE_VAR}" "${LIBRARY_VAR}" "${LINKAGE}" "${REQUIRED}")
    link_against_library("${NAME}" "${LINKAGE}" "${REQUIRED}")
endmacro()

macro(use_iconv LINKAGE REQUIRED)
    set(FORCE_EXTERNAL_ICONV OFF CACHE PATH "whether to force usage of external iconv (rather than the using the one bundled with glibc)")
    if(NOT FORCE_EXTERNAL_ICONV)
        # check whether iconv exists in standard lib
        include(CheckFunctionExists)
        check_function_exists(iconv HAS_ICONV)
    endif()
    if(NOT FORCE_EXTERNAL_ICONV AND HAS_ICONV)
        message(STATUS "Using iconv from the standard library for ${META_PROJECT_NAME}.")
    else()
        # find external iconv library
        use_external_library(iconv ${LINKAGE} ${REQUIRED})
    endif()
endmacro()
