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
        if("${REQUIRED}" STREQUAL "OPTIONAL")
            set(${NAME}_REQUIRED "NO")
        elseif("${REQUIRED}" STREQUAL "REQUIRED")
            set(${NAME}_REQUIRED "REQUIRED")
        else()
            message(FATAL_ERROR "Invalid use of use_external_library; must specify either REQUIRED or OPTIONAL.")
        endif()

        # add library to list of libraries to link against when building dynamic libraries or applications
        if(${NAME}_STATIC_LIB AND (("${LINKAGE}" STREQUAL "AUTO_LINKAGE" AND ((STATIC_LINKAGE AND "${META_PROJECT_TYPE}" STREQUAL "application") OR (STATIC_LIBRARY_LINKAGE AND ("${META_PROJECT_TYPE}" STREQUAL "" OR "${META_PROJECT_TYPE}" STREQUAL "library")))) OR ("${LINKAGE}" STREQUAL "STATIC")))
            list(APPEND LIBRARIES ${${NAME}_STATIC_LIB})
            message(STATUS "Linking ${META_PROJECT_NAME} statically against external library ${NAME} (${${NAME}_STATIC_LIB}).")
        elseif(${NAME}_DYNAMIC_LIB AND ("${LINKAGE}" STREQUAL "AUTO_LINKAGE" OR ("${LINKAGE}" STREQUAL "SHARED")))
            list(APPEND LIBRARIES ${${NAME}_DYNAMIC_LIB})
            message(STATUS "Linking ${META_PROJECT_NAME} dynamically against external library ${NAME} (${${NAME}_DYNAMIC_LIB}).")
        else()
            if(${NAME}_REQUIRED)
                message(FATAL_ERROR "External library ${NAME} required by ${META_PROJECT_NAME} is not available for the specified linkage ${LINKAGE}.")
            else()
                message(WARNING "External library ${NAME} required by ${META_PROJECT_NAME} is not available for the specified linkage ${LINKAGE}.")
            endif()
        endif()

        # add library to list of libraries to be provided as transitive dependencies when building static libraries
        list(APPEND STATIC_LIBRARIES ${${NAME}_STATIC_LIB})
    endmacro()

    macro(use_external_library_from_package NAME VERSION INCLUDE_VAR LIBRARY_VAR LINKAGE REQUIRED)
        save_default_library_suffixes()

        # handle specified VERSION
        if("${VERSION}" STREQUAL "ANY_VERSION")
            set(${NAME}_COMPATIBLE_VERSION "")
        else()
            set(${NAME}_COMPATIBLE_VERSION ${VERSION})
        endif()

        # find dynamic library
        configure_dynamic_library_suffixes()
        find_package(${NAME} ${${NAME}_COMPATIBLE_VERSION})
        include_directories(${${INCLUDE_VAR}})
        set(${NAME}_DYNAMIC_LIB ${${LIBRARY_VAR}})
        unset(${${LIBRARY_VAR}})

        # find static library
        configure_static_library_suffixes()
        find_package(${NAME} ${${NAME}_COMPATIBLE_VERSION})
        set(${NAME}_STATIC_LIB ${${LIBRARY_VAR}})

        link_against_library(${NAME} ${LINKAGE} ${REQUIRED})

        restore_default_library_suffixes()
    endmacro()

    macro(find_iconv LINKAGE REQUIRED)
        # check whether iconv exists in the standard library
        include(CheckFunctionExists)
        check_function_exists(iconv HAS_ICONV)
        if(HAS_ICONV)
            message(STATUS "Using iconv from the standard library for ${META_PROJECT_NAME}.")
        else()
            # find external iconv library
            save_default_library_suffixes()

            configure_dynamic_library_suffixes()
            find_library(ICONV_DYNAMIC_LIB iconv)

            configure_static_library_suffixes()
            find_library(ICONV_STATIC_LIB iconv)

            link_against_library(ICONV ${LINKAGE} ${REQUIRED})

            restore_default_library_suffixes()
        endif()
    endmacro()
endif()
