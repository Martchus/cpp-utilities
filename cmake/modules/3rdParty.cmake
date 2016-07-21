if(NOT DEFINED EXTERNAL_LIBRARIES_EXISTS)
    set(EXTERNAL_LIBRARIES_EXISTS true)
    macro(use_external_library_from_package NAME VERSION INCLUDE_VAR LIBRARY_VAR LINKAGE REQUIRED)
        # need to set CMAKE_FIND_LIBRARY_SUFFIXES temporarily to be able to find static libs, save the current value to be able to restore
        set(DEFAULT_CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})

        # handle arguments VERSION and REQUIRED
        if("${VERSION}" STREQUAL "ANY_VERSION")
            set(${NAME}_COMPATIBLE_VERSION "")
        else()
            set(${NAME}_COMPATIBLE_VERSION ${VERSION})
        endif()
        if("${REQUIRED}" STREQUAL "OPTIONAL")
            set(${NAME}_REQUIRED "")
        elseif("${REQUIRED}" STREQUAL "REQUIRED")
            set(${NAME}_REQUIRED "REQUIRED")
        else()
            message(FATAL_ERROR "Invalid use of use_external_library; must specify either REQUIRED or OPTIONAL.")
        endif()

        # find dynamic library
        if(WIN32)
            set(CMAKE_FIND_LIBRARY_SUFFIXES .so)
        else()
            set(CMAKE_FIND_LIBRARY_SUFFIXES .dll.a .dll)
        endif()
        find_package(${NAME} ${${NAME}_COMPATIBLE_VERSION})
        include_directories(${${INCLUDE_VAR}})
        set(${NAME}_DYNAMIC_LIB ${${LIBRARY_VAR}})
        set(${${LIBRARY_VAR}} "")

        # find static library
        if(WIN32)
            set(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a)
        else()
            set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
        endif()
        find_package(${NAME} ${${NAME}_COMPATIBLE_VERSION})
        set(${NAME}_STATIC_LIB ${${LIBRARY_VAR}})

        # add library to list of libraries to link against when building dynamic libraries or applications
        if(${NAME}_STATIC_LIB AND (("${LINKAGE}" STREQUAL "AUTO_LINKAGE" AND ((STATIC_LINKAGE AND "${META_PROJECT_TYPE}" STREQUAL "application") OR (STATIC_LIBRARY_LINKAGE AND ("${META_PROJECT_TYPE}" STREQUAL "" OR "${META_PROJECT_TYPE}" STREQUAL "library")))) OR ("${LINKAGE}" STREQUAL "STATIC")))
            list(APPEND LIBRARIES ${${NAME}_STATIC_LIB})
            message(STATUS "Linking ${META_PROJECT_NAME} statically against external library ${NAME}.")
        elseif(${NAME}_DYNAMIC_LIB AND ("${LINKAGE}" STREQUAL "AUTO_LINKAGE" OR ("${LINKAGE}" STREQUAL "SHARED")))
            list(APPEND LIBRARIES ${${NAME}_DYNAMIC_LIB})
            message(STATUS "Linking ${META_PROJECT_NAME} dynamically against external library ${NAME}.")
        else()
            if(${REQUIRED})
                message(FATAL_ERROR "External library ${NAME} required by ${META_PROJECT_NAME} is not available for the specified linkage ${LINKAGE}.")
            else()
                message(WARNING "External library ${NAME} required by ${META_PROJECT_NAME} is not available for the specified linkage ${LINKAGE}.")
            endif()
        endif()

        # add library to list of libraries to be provided as transitive dependencies when building static libraries
        list(APPEND STATIC_LIBRARIES ${${LIBRARY_VAR}})

        # restore altered CMAKE_FIND_LIBRARY_SUFFIXES
        set(CMAKE_FIND_LIBRARY_SUFFIXES ${DEFAULT_CMAKE_FIND_LIBRARY_SUFFIXES})
        set(DEFAULT_CMAKE_FIND_LIBRARY_SUFFIXES "")
    endmacro()
endif()
