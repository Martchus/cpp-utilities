cmake_minimum_required(VERSION 3.3.0 FATAL_ERROR)

# prevent multiple inclusion
if (DEFINED THIRD_PARTY_MODULE_LOADED)
    return()
endif ()
set(THIRD_PARTY_MODULE_LOADED YES)

macro (save_default_library_suffixes)
    set(DEFAULT_CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})
endmacro ()

macro (restore_default_library_suffixes)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ${DEFAULT_CMAKE_FIND_LIBRARY_SUFFIXES})
    unset(DEFAULT_CMAKE_FIND_LIBRARY_SUFFIXES)
endmacro ()

macro (configure_static_library_suffixes)
    # allows to look for static libraries in particular NOTE: code duplicated in Config.cmake.in
    if (WIN32)
        set(CMAKE_FIND_LIBRARY_SUFFIXES .a .lib)
    else ()
        set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
    endif ()
endmacro ()

macro (configure_dynamic_library_suffixes)
    # allows to look for dynamic libraries in particular
    if (WIN32)
        set(CMAKE_FIND_LIBRARY_SUFFIXES .dll .dll.a)
    elseif (APPLE)
        set(CMAKE_FIND_LIBRARY_SUFFIXES .dylib .so)
    else ()
        set(CMAKE_FIND_LIBRARY_SUFFIXES .so)
    endif ()
endmacro ()

function (validate_visibility VISIBILITY)
    if (NOT (VISIBILITY STREQUAL PUBLIC OR VISIBILITY STREQUAL PRIVATE))
        message(FATAL_ERROR "Specified visibility ${VISIBILITY} is invalid (must be either PUBLIC or PRIVATE).")
    endif ()
endfunction ()

function (parse_arguments_for_use_functions)
    # parse arguments
    set(OPTIONAL_ARGS OPTIONAL)
    set(ONE_VALUE_ARGS VISIBILITY LIBRARIES_VARIABLE PACKAGES_VARIABLE PKG_CONFIG_MODULES_VARIABLE TARGET_NAME)
    set(MULTI_VALUE_ARGS PKG_CONFIG_MODULES)
    cmake_parse_arguments(ARGS "${OPTIONAL_ARGS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" ${ARGN})

    # validate values
    if (ARGS_VISIBILITY)
        validate_visibility(${ARGS_VISIBILITY})
    else ()
        set(ARGS_VISIBILITY PRIVATE)
    endif ()

    if (NOT ARGS_LIBRARIES_VARIABLE)
        set(ARGS_LIBRARIES_VARIABLE "${ARGS_VISIBILITY}_LIBRARIES")
    endif ()
    if (NOT ARGS_PACKAGES_VARIABLE)
        if (NOT BUILD_SHARED_LIBS OR VISIBILITY STREQUAL PUBLIC)
            set(ARGS_PACKAGES_VARIABLE "INTERFACE_REQUIRED_PACKAGES")
        else ()
            set(ARGS_PACKAGES_VARIABLE "REQUIRED_PACKAGES")
        endif ()
    endif ()
    if (NOT ARGS_PKG_CONFIG_MODULES_VARIABLE)
        if (NOT BUILD_SHARED_LIBS OR VISIBILITY STREQUAL PUBLIC)
            set(ARGS_PKG_CONFIG_MODULES_VARIABLE "INTERFACE_REQUIRED_PKG_CONFIG_MODULES")
        else ()
            set(ARGS_PKG_CONFIG_MODULES_VARIABLE "REQUIRED_PKG_CONFIG_MODULES")
        endif ()
    endif ()

    # export parsed values to parent scope
    set(ARGS_VISIBILITY "${ARGS_VISIBILITY}" PARENT_SCOPE)
    set(ARGS_LIBRARIES_VARIABLE "${ARGS_LIBRARIES_VARIABLE}" PARENT_SCOPE)
    set(ARGS_PACKAGES_VARIABLE "${ARGS_PACKAGES_VARIABLE}" PARENT_SCOPE)
    set(ARGS_PKG_CONFIG_MODULES_VARIABLE "${ARGS_PKG_CONFIG_MODULES_VARIABLE}" PARENT_SCOPE)
    set(ARGS_TARGET_NAME "${ARGS_TARGET_NAME}" PARENT_SCOPE)
    set(ARGS_PKG_CONFIG_MODULES "${ARGS_PKG_CONFIG_MODULES}" PARENT_SCOPE)
    set(ARGS_OPTIONAL "${ARGS_OPTIONAL}" PARENT_SCOPE)
    if (NOT ARGS_OPTIONAL)
        set(ARGS_FIND_PACKAGE "REQUIRED" PARENT_SCOPE)
        set(ARGS_PKG_CHECK_MODULES "REQUIRED" PARENT_SCOPE)
    endif ()
endfunction ()

function (use_iconv)
    parse_arguments_for_use_functions(${ARGN})

    # check whether iconv from the standard library can be used
    set(FORCE_EXTERNAL_ICONV OFF
        CACHE PATH "whether to force usage of external iconv (rather than the using the one bundled with glibc)")
    if (NOT FORCE_EXTERNAL_ICONV)
        # check whether iconv exists in standard lib
        include(CheckFunctionExists)
        check_function_exists(iconv HAS_ICONV)
    endif ()
    if (NOT FORCE_EXTERNAL_ICONV AND HAS_ICONV)
        message(STATUS "Using iconv from the standard library for target ${META_PROJECT_NAME}.")
        return()
    endif ()

    # find external iconv library
    if (NOT TARGET Iconv::Iconv)
        set(Iconv_IS_BUILT_IN FALSE)
        find_package(Iconv ${ARGS_FIND_PACKAGE})
        if (NOT Iconv_FOUND)
            return()
        endif ()
    endif ()

    set("${ARGS_LIBRARIES_VARIABLE}" "${${ARGS_LIBRARIES_VARIABLE}};Iconv::Iconv" PARENT_SCOPE)
    set("${ARGS_PACKAGES_VARIABLE}" "${${ARGS_PACKAGES_VARIABLE}};Iconv" PARENT_SCOPE)
endfunction ()

function (use_openssl)
    parse_arguments_for_use_functions(${ARGN})

    find_package(OpenSSL ${ARGS_FIND_PACKAGE})
    if (NOT OpenSSL_FOUND)
        return()
    endif ()
    set("${ARGS_LIBRARIES_VARIABLE}" "${${ARGS_LIBRARIES_VARIABLE}};OpenSSL::SSL;OpenSSL::Crypto" PARENT_SCOPE)
    set("${ARGS_PACKAGES_VARIABLE}" "${${ARGS_PACKAGES_VARIABLE}};OpenSSL" PARENT_SCOPE)
    if (WIN32 AND OPENSSL_USE_STATIC_LIBS)
        set("${ARGS_LIBRARIES_VARIABLE}" "${${ARGS_LIBRARIES_VARIABLE}};-lws2_32;-lgdi32;-lcrypt32" PARENT_SCOPE)
    endif ()
    set("PKG_CONFIG_OpenSSL_SSL" "libssl" PARENT_SCOPE)
    set("PKG_CONFIG_OpenSSL_Crypto" "libcrypto" PARENT_SCOPE)
endfunction ()

function (use_crypto)
    parse_arguments_for_use_functions(${ARGN})

    find_package(OpenSSL ${ARGS_FIND_PACKAGE})
    if (NOT OpenSSL_FOUND)
        return()
    endif ()
    set("${ARGS_LIBRARIES_VARIABLE}" "${${ARGS_LIBRARIES_VARIABLE}};OpenSSL::Crypto" PARENT_SCOPE)
    set("${ARGS_PACKAGES_VARIABLE}" "${${ARGS_PACKAGES_VARIABLE}};OpenSSL" PARENT_SCOPE)
    if (WIN32 AND OPENSSL_USE_STATIC_LIBS)
        set("${ARGS_LIBRARIES_VARIABLE}" "${${ARGS_LIBRARIES_VARIABLE}};OpenSSL::Crypto;-lws2_32;-lgdi32;-lcrypt32"
            PARENT_SCOPE)
    endif ()
    set("PKG_CONFIG_OpenSSL_Crypto" "libcrypto" PARENT_SCOPE)
endfunction ()

function (use_zlib)
    parse_arguments_for_use_functions(${ARGN})

    find_package(ZLIB ${ARGS_FIND_PACKAGE})
    if (NOT ZLIB_FOUND)
        return()
    endif ()
    set("${ARGS_LIBRARIES_VARIABLE}" "${${ARGS_LIBRARIES_VARIABLE}};ZLIB::ZLIB" PARENT_SCOPE)
    set("${ARGS_PACKAGES_VARIABLE}" "${${ARGS_PACKAGES_VARIABLE}};ZLIB" PARENT_SCOPE)
    set("PKG_CONFIG_ZLIB_ZLIB" "zlib" PARENT_SCOPE)
endfunction ()

function (use_target)
    parse_arguments_for_use_functions(${ARGN})

    if (NOT TARGET "${ARGS_TARGET_NAME}")
        if (ARGS_OPTIONAL)
            return()
        endif ()
        message(FATAL_ERROR "Target \"${ARGS_TARGET_NAME}\" does not exist.")
    endif ()
    set("${ARGS_LIBRARIES_VARIABLE}" "${${ARGS_LIBRARIES_VARIABLE}};${ARGS_TARGET_NAME}" PARENT_SCOPE)
endfunction ()

function (use_pkg_config_module)
    # parse and validate arguments
    parse_arguments_for_use_functions(${ARGN})
    if (NOT ARGS_PKG_CONFIG_MODULES)
        message(FATAL_ERROR "No pkg-config modules specified.")
    endif ()
    if (NOT ARGS_TARGET_NAME)
        list(LENGTH ARGS_PKG_CONFIG_MODULES ARGS_PKG_CONFIG_MODULES_LENGTH)
        if (ARGS_PKG_CONFIG_MODULES_LENGTH STREQUAL 1)
            list(GET ARGS_PKG_CONFIG_MODULES 0 ARGS_TARGET_NAME)
        else ()
            message(FATAL_ERROR "No target name for multi-module pkg-config specified.")
        endif ()
    endif ()

    # skip if target has already been added
    if (TARGET "${ARGS_TARGET_NAME}")
        return()
    endif ()

    find_package(PkgConfig)
    pkg_check_modules(PKG_CHECK_MODULES_RESULT ${ARGS_PKG_CHECK_MODULES} ${ARGS_PKG_CONFIG_MODULES})

    # create interface library
    add_library(${ARGS_TARGET_NAME} INTERFACE IMPORTED)
    if (PKG_CONFIG_USE_STATIC_LIBS)
        set(PKG_CONFIG_CHECK_SUFFIX "_STATIC")
    else ()
        set(PKG_CONFIG_CHECK_SUFFIX "")
    endif ()
    set_property(TARGET ${ARGS_TARGET_NAME}
                 PROPERTY INTERFACE_LINK_LIBRARIES "${PKG_CHECK_MODULES_RESULT${PKG_CONFIG_CHECK_SUFFIX}_LINK_LIBRARIES}")
    set_property(TARGET ${ARGS_TARGET_NAME}
                 PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${PKG_CHECK_MODULES_RESULT${PKG_CONFIG_CHECK_SUFFIX}_INCLUDE_DIRS}")
    set_property(TARGET ${ARGS_TARGET_NAME}
                 PROPERTY INTERFACE_COMPILE_OPTIONS "${PKG_CHECK_MODULES_RESULT${PKG_CONFIG_CHECK_SUFFIX}_CFLAGS_OTHER}")
    set_property(TARGET ${ARGS_TARGET_NAME}
                 PROPERTY INTERFACE_LINK_OPTIONS "${PKG_CHECK_MODULES_RESULT${PKG_CONFIG_CHECK_SUFFIX}_LDFLAGS_OTHER}")

    set("${ARGS_PKG_CONFIG_MODULES_VARIABLE}" "${${ARGS_PKG_CONFIG_MODULES_VARIABLE}};${ARGS_TARGET_NAME}" PARENT_SCOPE)
    set("${ARGS_LIBRARIES_VARIABLE}" "${${ARGS_LIBRARIES_VARIABLE}};${ARGS_TARGET_NAME}" PARENT_SCOPE)
    string(REPLACE "::"
                   "_"
                   TARGET_VARNAME
                   "${ARGS_TARGET_NAME}")
    set("PKG_CONFIG_${TARGET_VARNAME}" "${ARGS_PKG_CONFIG_MODULES}" PARENT_SCOPE)
endfunction ()

# skip subsequent configuration if only the function includes are wanted
if (META_NO_3RDPARTY_CONFIG)
    return()
endif ()

# add options for deciding whether to build/use static or shared libraries
if (("${META_PROJECT_TYPE}" STREQUAL "library")
    OR ("${META_PROJECT_TYPE}" STREQUAL "plugin")
    OR ("${META_PROJECT_TYPE}" STREQUAL "qtplugin")
    OR ("${META_PROJECT_TYPE}" STREQUAL ""))
    set(META_PROJECT_IS_LIBRARY YES)
elseif ("${META_PROJECT_TYPE}" STREQUAL "application")
    set(META_PROJECT_IS_APPLICATION YES)
endif ()
if (META_PROJECT_IS_LIBRARY)
    option(BUILD_SHARED_LIBS ON "whether to build shared or static libraries")
    option(STATIC_LIBRARY_LINKAGE "adds flags for static linkage when building dynamic libraries" OFF)
elseif (META_PROJECT_IS_APPLICATION)
    option(STATIC_LINKAGE "adds flags for static linkage when building applications" OFF)
endif ()

# configure "static linkage"
if ((STATIC_LINKAGE AND META_PROJECT_IS_APPLICATION) OR (STATIC_LIBRARY_LINKAGE AND META_PROJECT_IS_LIBRARY))
    set(STATIC_LINKAGE_CONFIGURED ON)

    # add additional linker flags to achieve a fully statically linked build
    set(STATIC_LINKAGE_LINKER_FLAGS)
    if (NOT APPLE)
        list(APPEND STATIC_LINKAGE_LINKER_FLAGS -static)
    endif ()
    list(APPEND STATIC_LINKAGE_LINKER_FLAGS -static-libstdc++ -static-libgcc)

    if (META_PROJECT_IS_APPLICATION)
        list(APPEND META_ADDITIONAL_LINK_FLAGS ${STATIC_LINKAGE_LINKER_FLAGS})
    endif ()
    list(APPEND META_ADDITIONAL_LINK_FLAGS_TEST_TARGET ${STATIC_LINKAGE_LINKER_FLAGS})

    # prefer static libraries
    set(OPENSSL_USE_STATIC_LIBS ON)
    set(BOOST_USE_STATIC_LIBS ON)
    set(PKG_CONFIG_USE_STATIC_LIBS ON)
    configure_static_library_suffixes()

else ()
    set(STATIC_LINKAGE_CONFIGURED OFF)
endif ()
