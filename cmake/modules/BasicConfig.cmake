# before including this module, the project meta-data must be set

# set project name (displayed in Qt Creator)
message(STATUS "Configuring project ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}")
project(${META_PROJECT_NAME})

# set META_PROJECT_VARNAME and META_PROJECT_VARNAME_UPPER if not specified explicitely
if(NOT META_PROJECT_VARNAME)
    set(META_PROJECT_VARNAME "${META_PROJECT_NAME}")
endif()
if(NOT META_PROJECT_VARNAME_UPPER)
    string(TOUPPER ${META_PROJECT_VARNAME} META_PROJECT_VARNAME_UPPER)
endif()

# set META_GENERIC_NAME to META_APP_NAME if not specified explicitely
if(NOT META_GENERIC_NAME)
    set(META_GENERIC_NAME "${META_APP_NAME}")
endif()

# set default CXX_STANDARD for all library, application and test targets
if(NOT META_CXX_STANDARD)
    set(META_CXX_STANDARD 14)
endif()

# provide variables for other projects built as part of the same subdirs project
# to access files from this project
get_directory_property(HAS_PARENT PARENT_DIRECTORY)
if(HAS_PARENT)
    set(${META_PROJECT_VARNAME_UPPER}_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" PARENT_SCOPE)
    set(${META_PROJECT_VARNAME_UPPER}_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}" PARENT_SCOPE)
    set(${META_PROJECT_NAME}_DIR "${CMAKE_CURRENT_BINARY_DIR}" PARENT_SCOPE)
    if(CMAKE_FIND_ROOT_PATH AND MINGW)
        set(RUNTIME_LIBRARY_PATH "${CMAKE_CURRENT_BINARY_DIR}" ${RUNTIME_LIBRARY_PATH} PARENT_SCOPE)
    endif()
endif()

# stringify the meta data
set(META_PROJECT_NAME_STR "\"${META_PROJECT_NAME}\"")
set(META_APP_VERSION ${META_VERSION_MAJOR}.${META_VERSION_MINOR}.${META_VERSION_PATCH})
set(META_APP_NAME_STR "\"${META_APP_NAME}\"")
set(META_APP_AUTHOR_STR "\"${META_APP_AUTHOR}\"")
set(META_APP_URL_STR "\"${META_APP_URL}\"")
set(META_APP_DESCRIPTION_STR "\"${META_APP_DESCRIPTION}\"")
set(META_APP_VERSION_STR "\"${META_APP_VERSION}\"")

# set TARGET_EXECUTABLE which is used to refer to the target executable at its installation location
set(TARGET_EXECUTABLE "${CMAKE_INSTALL_PREFIX}/bin/${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}")

# disable new ABI (can't catch ios_base::failure with new ABI)
option(FORCE_OLD_ABI "specifies whether usage of old ABI should be forced" OFF)
if(FORCE_OLD_ABI)
    list(APPEND META_PRIVATE_COMPILE_DEFINITIONS _GLIBCXX_USE_CXX11_ABI=0)
    message(STATUS "Forcing usage of old CXX11 ABI.")
else()
    message(STATUS "Using default CXX11 ABI (not forcing old CX11 ABI).")
endif()

# enable debug-only code when doing a debug build
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND META_PRIVATE_COMPILE_DEFINITIONS DEBUG_BUILD)
    message(STATUS "Debug build enabled.")
endif()

# enable logging when option is set
option(LOGGING_ENABLED "specifies whether logging is enabled" OFF)
if(LOGGING_ENABLED)
    list(APPEND META_PRIVATE_COMPILE_DEFINITIONS LOGGING_ENABLED)
    message(STATUS "Logging is enabled.")
endif()

# options for deciding whether to build static and/or shared libraries
if(("${META_PROJECT_TYPE}" STREQUAL "library") OR ("${META_PROJECT_TYPE}" STREQUAL ""))
    option(ENABLE_STATIC_LIBS "whether building static libraries is enabled (disabled by default)" OFF)
    option(DISABLE_SHARED_LIBS "whether building dynamic libraries is disabled (enabled by default)" OFF)
    if(DISABLE_SHARED_LIBS)
        set(BUILD_SHARED_LIBS OFF)
    else()
        set(BUILD_SHARED_LIBS ON)
    endif()
    if(ENABLE_STATIC_LIBS)
        set(BUILD_STATIC_LIBS ON)
    else()
        set(BUILD_STATIC_LIBS OFF)
    endif()
endif()

# options for forcing static linkage when building applications or dynamic libraries
if(("${META_PROJECT_TYPE}" STREQUAL "library") OR ("${META_PROJECT_TYPE}" STREQUAL ""))
    option(STATIC_LIBRARY_LINKAGE "forces static linkage when building dynamic libraries" OFF)
elseif("${META_PROJECT_TYPE}" STREQUAL "application")
    option(STATIC_LINKAGE "forces static linkage when building applications" OFF)
endif()

# additional linker flags used when static linkage is enables
list(APPEND META_ADDITIONAL_STATIC_LINK_FLAGS -static -static-libstdc++ -static-libgcc)

# options for enabling/disabling Qt GUI (if available)
if(WIDGETS_HEADER_FILES OR WIDGETS_SRC_FILES OR WIDGETS_UI_FILES)
    if(META_GUI_OPTIONAL)
        option(WIDGETS_GUI "enables/disables building the Qt Widgets GUI: yes (default) or no" ON)
    else()
        set(WIDGETS_GUI ON)
    endif()
else()
    set(WIDGETS_GUI OFF)
endif()
if(QML_HEADER_FILES OR QML_SRC_FILES)
    if(META_GUI_OPTIONAL)
        option(QUICK_GUI "enables/disables building the Qt Quick GUI: yes (default) or no" ON)
    else()
        set(QUICK_GUI ON)
    endif()
else()
    set(QUICK_GUI OFF)
endif()
