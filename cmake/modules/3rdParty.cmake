if(NOT DEFINED FIND_THIRD_PARTY_LIBRARIES_EXISTS)
    set(FIND_THIRD_PARTY_LIBRARIES_EXISTS true)

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
        else()
            set(CMAKE_FIND_LIBRARY_SUFFIXES .so)
        endif()
    endmacro()

    macro(link_against_library NAME LINKAGE REQUIRED)
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
        if(${NAME}_STATIC_LIB AND (("${LINKAGE}" STREQUAL "AUTO_LINKAGE" AND ((STATIC_LINKAGE AND "${META_PROJECT_TYPE}" STREQUAL "application") OR (STATIC_LIBRARY_LINKAGE AND ("${META_PROJECT_TYPE}" STREQUAL "" OR "${META_PROJECT_TYPE}" STREQUAL "library")))) OR ("${LINKAGE}" STREQUAL "STATIC")))
            list(APPEND LIBRARIES ${${NAME}_STATIC_LIB})
            set(USE_STATIC_${NAME} ON)
            message(STATUS "Linking ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX} statically against external library ${NAME} (${${NAME}_STATIC_LIB}).")
        elseif(${NAME}_DYNAMIC_LIB AND ("${LINKAGE}" STREQUAL "AUTO_LINKAGE" OR ("${LINKAGE}" STREQUAL "SHARED")))
            list(APPEND LIBRARIES ${${NAME}_DYNAMIC_LIB})
            set(USE_STATIC_${NAME} OFF)
            message(STATUS "Linking ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX} dynamically against external library ${NAME} (${${NAME}_DYNAMIC_LIB}).")
        else()
            if(${NAME}_REQUIRED)
                message(FATAL_ERROR "External library ${NAME} required by ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX} is not available for the specified linkage ${LINKAGE}.")
            else()
                message(WARNING "External library ${NAME} required by ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX} is not available for the specified linkage ${LINKAGE}.")
            endif()
        endif()

        # add library to list of libraries to be provided as transitive dependencies when building static libraries
        if(${NAME}_STATIC_LIB)
            list(APPEND STATIC_LIBRARIES ${${NAME}_STATIC_LIB})
            message(STATUS "Adding ${${NAME}_STATIC_LIB} to static library dependencies of ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}.")
        endif()
    endmacro()

    macro(use_external_library NAME LINKAGE REQUIRED)
        save_default_library_suffixes()
        configure_dynamic_library_suffixes()
        find_library(${NAME}_DYNAMIC_LIB ${NAME})
        configure_static_library_suffixes()
        find_library(${NAME}_STATIC_LIB ${NAME})
        link_against_library(${NAME} ${LINKAGE} ${REQUIRED})
        restore_default_library_suffixes()
    endmacro()

    function(use_external_library_from_package_dynamic NAME PKGNAME INCLUDE_VAR LIBRARY_VAR COMPAT_VERSION)
        # internally used by use_external_library_from_package to find dynamic libraries
        configure_dynamic_library_suffixes()
        find_package(${PKGNAME} ${COMPAT_VERSION})
        include_directories(${${INCLUDE_VAR}})
        set(${NAME}_DYNAMIC_LIB ${${LIBRARY_VAR}} PARENT_SCOPE)
    endfunction()

    function(use_external_library_from_package_static NAME PKGNAME INCLUDE_VAR LIBRARY_VAR COMPAT_VERSION)
        # internally used by use_external_library_from_package to find static libraries
        configure_static_library_suffixes()
        find_package(${PKGNAME} ${COMPAT_VERSION})
        include_directories(${${INCLUDE_VAR}})
        set(${NAME}_STATIC_LIB ${${LIBRARY_VAR}} PARENT_SCOPE)
    endfunction()

    macro(use_external_library_from_package NAME PKGNAME VERSION INCLUDE_VAR LIBRARY_VAR LINKAGE REQUIRED)
        save_default_library_suffixes()

        # handle specified VERSION
        if("${VERSION}" STREQUAL "ANY_VERSION")
            set(${NAME}_COMPATIBLE_VERSION "")
        else()
            set(${NAME}_COMPATIBLE_VERSION ${VERSION})
        endif()

        # use the find_library approach first because it is less buggy when trying to detect static libraries
        configure_dynamic_library_suffixes()
        find_library(${NAME}_DYNAMIC_LIB ${NAME})
        configure_static_library_suffixes()
        find_library(${NAME}_STATIC_LIB ${NAME})

        # fall back to actual use of find_package
        # use separate functions to get a new scope
        if(NOT ${NAME}_DYNAMIC_LIB)
            use_external_library_from_package_dynamic(${NAME} ${PKGNAME} ${INCLUDE_VAR} "${LIBRARY_VAR}" "${${NAME}_COMPATIBLE_VERSION}")
        endif()
        if(NOT ${NAME}_STATIC_LIB)
            use_external_library_from_package_static(${NAME} ${PKGNAME} ${INCLUDE_VAR} "${LIBRARY_VAR}" "${${NAME}_COMPATIBLE_VERSION}")
        endif()

        link_against_library(${NAME} ${LINKAGE} ${REQUIRED})

        restore_default_library_suffixes()
    endmacro()

    macro(use_iconv LINKAGE REQUIRED)
        # check whether iconv exists in the standard library
        include(CheckFunctionExists)
        check_function_exists(iconv HAS_ICONV)
        if(HAS_ICONV)
            message(STATUS "Using iconv from the standard library for ${META_PROJECT_NAME}.")
        else()
            # find external iconv library
            use_external_library(iconv ${LINKAGE} ${REQUIRED})
        endif()
    endmacro()
endif()
