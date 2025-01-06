cmake_minimum_required(VERSION 3.17.0 FATAL_ERROR)

# prevent multiple inclusion
if (DEFINED THIRD_PARTY_MODULE_LOADED)
    return()
endif ()
set(THIRD_PARTY_MODULE_LOADED YES)

include(3rdPartyFunctions)

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

    # add options to opt out from linking statically against certain core libraries as it might not work under all platforms
    # (see https://github.com/Martchus/syncthingtray/issues/64)
    if (APPLE OR UNIX)
        set(DEFAUT_NO_STATIC_ENFORCE ON)
        # note: The -static flag is considered completely unsupported under Apple platforms (see
        # https://stackoverflow.com/questions/844819/how-to-static-link-on-os-x) and generally not a good idea to use under
        # GNU/Linux as it leads to linking statically against glibc.
    else ()
        set(DEFAULT_NO_STATIC_ENFORCE OFF)
    endif ()
    option(NO_STATIC_ENFORCE "prevent enforcing static linkage despite generally aiming for a statically linked build"
           "${DEFAUT_NO_STATIC_ENFORCE}")
    option(NO_STATIC_LIBGCC
           "prevent linking statically against libgcc despite aiming otherwise for a statically linked build" OFF)
    option(NO_STATIC_LIBSTDCXX
           "prevent linking statically against libstdc++ despite aiming otherwise for a statically linked build" OFF)

    # add additional linker flags to achieve a fully statically linked build
    set(STATIC_LINKAGE_LINKER_FLAGS)
    if (NOT NO_STATIC_ENFORCE)
        list(APPEND STATIC_LINKAGE_LINKER_FLAGS -static)
    endif ()
    if (CMAKE_CXX_COMPILER_ID STREQUAL Clang AND (NOT NO_STATIC_LIBGCC OR NOT NO_STATIC_LIBSTDCXX))
        list(APPEND STATIC_LINKAGE_LINKER_FLAGS --start-no-unused-arguments)
    endif ()
    if (NOT NO_STATIC_LIBGCC)
        list(APPEND STATIC_LINKAGE_LINKER_FLAGS -static-libgcc)
    endif ()
    if (NOT NO_STATIC_LIBSTDCXX)
        list(APPEND STATIC_LINKAGE_LINKER_FLAGS -static-libstdc++)
    endif ()
    if (CMAKE_CXX_COMPILER_ID STREQUAL Clang AND (NOT NO_STATIC_LIBGCC OR NOT NO_STATIC_LIBSTDCXX))
        list(APPEND STATIC_LINKAGE_LINKER_FLAGS --end-no-unused-arguments)
    endif ()
    if (META_PROJECT_IS_APPLICATION)
        list(APPEND META_ADDITIONAL_LINK_FLAGS ${STATIC_LINKAGE_LINKER_FLAGS})
    endif ()
    list(APPEND META_ADDITIONAL_LINK_FLAGS_TEST_TARGET ${STATIC_LINKAGE_LINKER_FLAGS})

    prefer_static_libraries()
else ()
    set(STATIC_LINKAGE_CONFIGURED OFF)
endif ()
