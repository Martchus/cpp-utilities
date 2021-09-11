cmake_minimum_required(VERSION 3.3.0 FATAL_ERROR)

# prevent multiple inclusion
if (DEFINED THIRD_PARTY_MODULE_LOADED)
    return()
endif ()
set(THIRD_PARTY_MODULE_LOADED YES)

cmake_policy(SET CMP0067 NEW) # make check_cxx_source_compiles() pick up the variables for the C++ version
include(CheckCXXSourceCompiles)

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
    set(OPTIONAL_ARGS OPTIONAL ONLY_HEADERS)
    set(ONE_VALUE_ARGS VISIBILITY LIBRARIES_VARIABLE PACKAGES_VARIABLE PKG_CONFIG_MODULES_VARIABLE TARGET_NAME PACKAGE_NAME)
    set(MULTI_VALUE_ARGS PKG_CONFIG_MODULES PACKAGE_ARGS)
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
        if (NOT BUILD_SHARED_LIBS OR ARGS_VISIBILITY STREQUAL PUBLIC)
            set(ARGS_PACKAGES_VARIABLE "INTERFACE_REQUIRED_PACKAGES")
        else ()
            set(ARGS_PACKAGES_VARIABLE "REQUIRED_PACKAGES")
        endif ()
    endif ()
    if (NOT ARGS_PKG_CONFIG_MODULES_VARIABLE)
        if (NOT BUILD_SHARED_LIBS OR ARGS_VISIBILITY STREQUAL PUBLIC)
            set(ARGS_PKG_CONFIG_MODULES_VARIABLE "INTERFACE_REQUIRED_PKG_CONFIG_MODULES")
        else ()
            set(ARGS_PKG_CONFIG_MODULES_VARIABLE "REQUIRED_PKG_CONFIG_MODULES")
        endif ()
    endif ()

    # export parsed values to parent scope
    set(ARGS_VISIBILITY
        "${ARGS_VISIBILITY}"
        PARENT_SCOPE)
    set(ARGS_LIBRARIES_VARIABLE
        "${ARGS_LIBRARIES_VARIABLE}"
        PARENT_SCOPE)
    set(ARGS_PACKAGES_VARIABLE
        "${ARGS_PACKAGES_VARIABLE}"
        PARENT_SCOPE)
    set(ARGS_PKG_CONFIG_MODULES_VARIABLE
        "${ARGS_PKG_CONFIG_MODULES_VARIABLE}"
        PARENT_SCOPE)
    set(ARGS_TARGET_NAME
        "${ARGS_TARGET_NAME}"
        PARENT_SCOPE)
    set(ARGS_PACKAGE_NAME
        "${ARGS_PACKAGE_NAME}"
        PARENT_SCOPE)
    set(ARGS_PACKAGE_ARGS
        "${ARGS_PACKAGE_ARGS}"
        PARENT_SCOPE)
    set(ARGS_PKG_CONFIG_MODULES
        "${ARGS_PKG_CONFIG_MODULES}"
        PARENT_SCOPE)
    set(ARGS_OPTIONAL
        "${ARGS_OPTIONAL}"
        PARENT_SCOPE)
    set(ARGS_ONLY_HEADERS
        "${ARGS_ONLY_HEADERS}"
        PARENT_SCOPE)
    if (NOT ARGS_OPTIONAL)
        set(ARGS_FIND_PACKAGE
            "REQUIRED"
            PARENT_SCOPE)
        set(ARGS_PKG_CHECK_MODULES
            "REQUIRED"
            PARENT_SCOPE)
    endif ()
endfunction ()

function (use_iconv)
    parse_arguments_for_use_functions(${ARGN})

    # check whether iconv from the standard library can be used
    set(FORCE_EXTERNAL_ICONV
        OFF
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

    set("${ARGS_LIBRARIES_VARIABLE}"
        "${${ARGS_LIBRARIES_VARIABLE}};Iconv::Iconv"
        PARENT_SCOPE)
    set("${ARGS_PACKAGES_VARIABLE}"
        "${${ARGS_PACKAGES_VARIABLE}};Iconv"
        PARENT_SCOPE)
endfunction ()

macro (_cpp_utilities_use_openssl OPENSSL_TARGETS)
    find_package(OpenSSL ${ARGS_FIND_PACKAGE})
    if (NOT OpenSSL_FOUND)
        message(STATUS "Unable to find OpenSSL")
        return()
    endif ()
    foreach (OPENSSL_TARGET ${OPENSSL_TARGETS})
        if (TARGET "${OPENSSL_TARGET}")
            continue()
        endif ()
        set(MESSAGE_MODE WARNING)
        if (REQUIRED IN_LIST ARGS_FIND_PACKAGE)
            set(MESSAGE_MODE FATAL_ERROR)
        endif ()
        message(
            "${MESSAGE_MODE}"
            "Found OpenSSL but imported target \"${OPENSSL_TARGET}\" is missing. Possibly the devel package for OpenSSL is not installed."
        )
        return()
    endforeach ()

    message(STATUS "Found required OpenSSL targets (${OPENSSL_TARGETS})")
    set("${ARGS_LIBRARIES_VARIABLE}" "${${ARGS_LIBRARIES_VARIABLE}};${OPENSSL_TARGETS}")
    if (WIN32 AND OPENSSL_USE_STATIC_LIBS)
        # FIXME: preferably use pkg-config to cover this case without hardcoding OpenSSL's dependencies under Windows
        set("${ARGS_LIBRARIES_VARIABLE}" "${${ARGS_LIBRARIES_VARIABLE}};-lws2_32;-lgdi32;-lcrypt32")
    endif ()
    set("${ARGS_PACKAGES_VARIABLE}"
        "${${ARGS_PACKAGES_VARIABLE}};OpenSSL"
        PARENT_SCOPE)
    set("${ARGS_LIBRARIES_VARIABLE}"
        "${${ARGS_LIBRARIES_VARIABLE}}"
        PARENT_SCOPE)
    set("PKG_CONFIG_OpenSSL_Crypto"
        "libcrypto"
        PARENT_SCOPE)
endmacro ()

function (use_openssl)
    parse_arguments_for_use_functions(${ARGN})
    _cpp_utilities_use_openssl("OpenSSL::SSL;OpenSSL::Crypto")
    set("PKG_CONFIG_OpenSSL_SSL"
        "libssl"
        PARENT_SCOPE)

endfunction ()

function (use_crypto)
    parse_arguments_for_use_functions(${ARGN})
    _cpp_utilities_use_openssl("OpenSSL::Crypto")
endfunction ()

function (use_zlib)
    parse_arguments_for_use_functions(${ARGN})

    find_package(ZLIB ${ARGS_FIND_PACKAGE})
    if (NOT ZLIB_FOUND)
        return()
    endif ()
    set("${ARGS_LIBRARIES_VARIABLE}"
        "${${ARGS_LIBRARIES_VARIABLE}};ZLIB::ZLIB"
        PARENT_SCOPE)
    set("${ARGS_PACKAGES_VARIABLE}"
        "${${ARGS_PACKAGES_VARIABLE}};ZLIB"
        PARENT_SCOPE)
    set("PKG_CONFIG_ZLIB_ZLIB"
        "zlib"
        PARENT_SCOPE)
endfunction ()

function (use_target)
    parse_arguments_for_use_functions(${ARGN})

    if (NOT TARGET "${ARGS_TARGET_NAME}")
        if (ARGS_OPTIONAL)
            return()
        endif ()
        message(FATAL_ERROR "Target \"${ARGS_TARGET_NAME}\" does not exist.")
    endif ()
    set("${ARGS_LIBRARIES_VARIABLE}"
        "${${ARGS_LIBRARIES_VARIABLE}};${ARGS_TARGET_NAME}"
        PARENT_SCOPE)
    if (ARGS_PACKAGE_NAME)
        set("${ARGS_PACKAGES_VARIABLE}"
            "${${ARGS_PACKAGES_VARIABLE}};${ARGS_PACKAGE_NAME}"
            PARENT_SCOPE)
        if (ARGS_PACKAGE_ARGS)
            set("PACKAGE_ARGS_${ARGS_PACKAGE_NAME}"
                "${ARGS_PACKAGE_ARGS}"
                PARENT_SCOPE)
        endif ()
    endif ()
endfunction ()

function (use_package)
    parse_arguments_for_use_functions(${ARGN})

    if (NOT ARGS_PACKAGE_NAME)
        message(FATAL_ERROR "No PACKAGE_NAME specified.")
    endif ()
    if (NOT ARGS_TARGET_NAME)
        message(FATAL_ERROR "No TARGET_NAME specified.")
    endif ()
    if (NOT ARGS_PACKAGE_ARGS)
        set(ARGS_PACKAGE_ARGS ${ARGS_FIND_PACKAGE})
    endif ()

    find_package("${ARGS_PACKAGE_NAME}" ${ARGS_PACKAGE_ARGS})
    if (NOT TARGET "${ARGS_TARGET_NAME}")
        if (ARGS_OPTIONAL)
            return()
        endif ()
        message(FATAL_ERROR "Found package \"${ARGS_PACKAGE_NAME}\" but target \"${ARGS_TARGET_NAME}\" does not exist.")
    endif ()
    set("${ARGS_LIBRARIES_VARIABLE}"
        "${${ARGS_LIBRARIES_VARIABLE}};${ARGS_TARGET_NAME}"
        PARENT_SCOPE)
    set("${ARGS_PACKAGES_VARIABLE}"
        "${${ARGS_PACKAGES_VARIABLE}};${ARGS_PACKAGE_NAME}"
        PARENT_SCOPE)
    set("PACKAGE_ARGS_${ARGS_PACKAGE_NAME}"
        "${ARGS_PACKAGE_ARGS}"
        PARENT_SCOPE)
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

    # add target only if it has not already been added; otherwise just add the existing target to the library variable
    if (NOT TARGET "${ARGS_TARGET_NAME}")
        find_package(PkgConfig)
        pkg_check_modules(PKG_CHECK_MODULES_RESULT ${ARGS_PKG_CHECK_MODULES} ${ARGS_PKG_CONFIG_MODULES})

        # create interface library
        add_library(${ARGS_TARGET_NAME} INTERFACE IMPORTED)
        if (PKG_CONFIG_USE_STATIC_LIBS)
            set(PKG_CONFIG_CHECK_SUFFIX "_STATIC")
        else ()
            set(PKG_CONFIG_CHECK_SUFFIX "")
        endif ()
        set_property(
            TARGET ${ARGS_TARGET_NAME} PROPERTY INTERFACE_LINK_LIBRARIES
                                                "${PKG_CHECK_MODULES_RESULT${PKG_CONFIG_CHECK_SUFFIX}_LINK_LIBRARIES}")
        set_property(
            TARGET ${ARGS_TARGET_NAME} PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                                                "${PKG_CHECK_MODULES_RESULT${PKG_CONFIG_CHECK_SUFFIX}_INCLUDE_DIRS}")
        set_property(TARGET ${ARGS_TARGET_NAME}
                     PROPERTY INTERFACE_COMPILE_OPTIONS "${PKG_CHECK_MODULES_RESULT${PKG_CONFIG_CHECK_SUFFIX}_CFLAGS_OTHER}")
        set_property(TARGET ${ARGS_TARGET_NAME}
                     PROPERTY INTERFACE_LINK_OPTIONS "${PKG_CHECK_MODULES_RESULT${PKG_CONFIG_CHECK_SUFFIX}_LDFLAGS_OTHER}")
    endif ()

    set("${ARGS_PKG_CONFIG_MODULES_VARIABLE}"
        "${${ARGS_PKG_CONFIG_MODULES_VARIABLE}};${ARGS_TARGET_NAME}"
        PARENT_SCOPE)
    set("${ARGS_LIBRARIES_VARIABLE}"
        "${${ARGS_LIBRARIES_VARIABLE}};${ARGS_TARGET_NAME}"
        PARENT_SCOPE)
    string(REPLACE "::" "_" TARGET_VARNAME "${ARGS_TARGET_NAME}")
    set("PKG_CONFIG_${TARGET_VARNAME}"
        "${ARGS_PKG_CONFIG_MODULES}"
        PARENT_SCOPE)
endfunction ()

function (use_standard_filesystem)
    # parse and validate arguments
    parse_arguments_for_use_functions(${ARGN})

    # set c++ standard for `check_cxx_source_compiles()`
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED 17)

    # check whether an additional library for std::filesystem support is required
    set(TEST_PROGRAM
        [[
        #include <chrono>
        #include <system_error>
        #include <filesystem>
        int main() {
            auto ec = std::error_code();
            const auto cwd = std::filesystem::current_path();
            const auto t = std::filesystem::last_write_time(cwd, ec);
            std::filesystem::last_write_time(cwd, t, ec);
            return static_cast<int>(cwd.string().size());
        }
    ]])
    set(DEFAULT_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
    set(REQUIRED_LIBRARY FAILED)
    set(INDEX 0)
    foreach (LIBRARY "" "stdc++fs" "c++fs")
        if (NOT LIBRARY STREQUAL "")
            set(CMAKE_REQUIRED_LIBRARIES ${DEFAULT_REQUIRED_LIBRARIES} -l${LIBRARY})
        endif ()
        check_cxx_source_compiles("${TEST_PROGRAM}" STD_FILESYSTEM_TEST_${INDEX})
        if (STD_FILESYSTEM_TEST_${INDEX})
            set(REQUIRED_LIBRARY "${LIBRARY}")
            break()
        endif ()
        math(EXPR INDEX "${INDEX}+1")
    endforeach ()

    # handle error
    if (REQUIRED_LIBRARY STREQUAL "FAILED")
        message(
            FATAL_ERROR
                "Unable to compile a simple std::filesystem example. A compiler supporting C++17 is required to build this project."
        )
        return()
    endif ()

    # handle case when no library is required
    if (REQUIRED_LIBRARY STREQUAL "")
        message(STATUS "Linking ${META_PROJECT_NAME} against special library for std::filesystem support is not required.")
        return()
    endif ()

    # prefer the static version of the library because the ABI might not be stable (note: stdc++fs seems to be only available
    # as static lib anyways)
    configure_static_library_suffixes()
    set(USED_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})
    find_library(STANDARD_FILE_SYSTEM_LIBRARY "${REQUIRED_LIBRARY}")
    configure_dynamic_library_suffixes()
    if (NOT STANDARD_FILE_SYSTEM_LIBRARY)
        # fallback to using -l if the library is hidden in some sub directory
        set(STANDARD_FILE_SYSTEM_LIBRARY "-l${REQUIRED_LIBRARY}")
    endif ()

    message(
        STATUS
            "Linking ${META_PROJECT_NAME} against library \"${STANDARD_FILE_SYSTEM_LIBRARY}\" for std::filesystem support.")
    set("${ARGS_LIBRARIES_VARIABLE}"
        "${${ARGS_LIBRARIES_VARIABLE}};${STANDARD_FILE_SYSTEM_LIBRARY}"
        PARENT_SCOPE)
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
    # when development defaults are enabled, build shared libs by default when targeting GNU/Linux
    set(BUILD_SHARED_LIBS_BY_DEFAULT "${BUILD_SHARED_LIBS}")
    if (ENABLE_DEVEL_DEFAULTS AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(BUILD_SHARED_LIBS_BY_DEFAULT ON)
    endif ()
    if (DEFINED CACHE{${META_PROJECT_VARNAME}_BUILD_SHARED_LIBS})
        # allow overriding BUILD_SHARED_LIBS via a project-specific cache variable
        set(BUILD_SHARED_LIBS "${${META_PROJECT_VARNAME}_BUILD_SHARED_LIBS}")
    else ()
        # make BUILD_SHARED_LIBS an overridable cache variable
        option(BUILD_SHARED_LIBS "whether to build shared or static libraries" "${BUILD_SHARED_LIBS_BY_DEFAULT}")
    endif ()
    option(
        STATIC_LIBRARY_LINKAGE
        "prefer linking against dependencies statically; adds additional flags for static linkage; only applies when building shared libraries"
        OFF)
elseif (META_PROJECT_IS_APPLICATION)
    option(
        STATIC_LINKAGE
        "prefer linking against dependencies statically; adds additional flags for static linkage; only applies when building applications"
        OFF)
endif ()

# configure "static linkage"
if ((STATIC_LINKAGE AND META_PROJECT_IS_APPLICATION) OR (STATIC_LIBRARY_LINKAGE AND META_PROJECT_IS_LIBRARY))
    set(STATIC_LINKAGE_CONFIGURED ON)

    # add options to opt out from linking statically against the C and C++ standard library as it might not work under all
    # platforms (see https://github.com/Martchus/syncthingtray/issues/64)
    option(NO_STATIC_LIBGCC
           "prevent linking statically against libgcc despite aiming otherwise for a statically linked build" OFF)
    option(NO_STATIC_LIBSTDCXX
           "prevent linking statically against libstdc++ despite aiming otherwise for a statically linked build" OFF)

    # add additional linker flags to achieve a fully statically linked build
    set(STATIC_LINKAGE_LINKER_FLAGS)
    if (NOT APPLE)
        list(APPEND STATIC_LINKAGE_LINKER_FLAGS -static)
        # note: The -static flag is considered completely unsupported under Apple platforms (see
        # https://stackoverflow.com/questions/844819/how-to-static-link-on-os-x).
    endif ()
    if (NOT NO_STATIC_LIBGCC)
        list(APPEND STATIC_LINKAGE_LINKER_FLAGS -static-libgcc)
    endif ()
    if (NOT NO_STATIC_LIBSTDCXX)
        list(APPEND STATIC_LINKAGE_LINKER_FLAGS -static-libstdc++)
    endif ()

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
