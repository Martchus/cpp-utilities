cmake_minimum_required(VERSION 3.17.0 FATAL_ERROR)

# generates and adds a Windows rc file for the application/library also attaches the application icon if ffmpeg is available
# does nothing if not building with mingw-w64

if (NOT BASIC_PROJECT_CONFIG_DONE)
    message(
        FATAL_ERROR
            "Before including the WindowsResources module, the LibraryConfig/ApplicationConfig module must be included.")
endif ()

option(WINDOWS_RESOURCES_ENABLED "controls whether Windows resources are enabled" ON)
option(WINDOWS_ICON_ENABLED "controls whether Windows icon is enabled" ON)

if (NOT WIN32 OR NOT WINDOWS_RESOURCES_ENABLED)
    return()
endif ()

# find rc template, define path of output rc file
include(TemplateFinder)
find_template_file("windows.rc" CPP_UTILITIES RC_TEMPLATE_FILE)
find_template_file("windows-cli-wrapper.rc" CPP_UTILITIES RC_CLI_TEMPLATE_FILE)
set(WINDOWS_RC_FILE "${CMAKE_CURRENT_BINARY_DIR}/resources/windows")
set(WINDOWS_CLI_RC_FILE "${CMAKE_CURRENT_BINARY_DIR}/resources/windows-cli-wrapper")

# create Windows icon from png with ffmpeg if available
unset(WINDOWS_ICON_RC_ENTRY)
if (WINDOWS_ICON_ENABLED)
    if (NOT WINDOWS_ICON_PATH AND PNG_ICON_PATH)
        find_program(FFMPEG_BIN ffmpeg avconv)
        if (FFMPEG_BIN)
            set(WINDOWS_ICON_PATH "${CMAKE_CURRENT_BINARY_DIR}/resources/${META_PROJECT_NAME}.ico")
            add_custom_command(
                COMMENT "Generating icon for Windows executable"
                OUTPUT "${WINDOWS_ICON_PATH}"
                COMMAND ${FFMPEG_BIN} -y -i "${PNG_ICON_PATH}" "${WINDOWS_ICON_PATH}"
                DEPENDS "${PNG_ICON_PATH}")
            message(STATUS "Generating Windows icon from \"${PNG_ICON_PATH}\" via ${FFMPEG_BIN}.")
        else ()
            message(STATUS "Unable to find ffmpeg, not creating a Windows icon")
        endif ()
    endif ()
    if (WINDOWS_ICON_PATH)
        set(WINDOWS_ICON_RC_ENTRY "IDI_ICON1   ICON    DISCARDABLE \"${WINDOWS_ICON_PATH}\"")
        set_source_files_properties("${WINDOWS_RC_FILE}" PROPERTIES OBJECT_DEPENDS "${WINDOWS_ICON_PATH}")
    endif ()
endif ()

# create Windows rc file from template
configure_file("${RC_TEMPLATE_FILE}" "${WINDOWS_RC_FILE}-configured.rc")
file(
    GENERATE
    OUTPUT "${WINDOWS_RC_FILE}-$<CONFIG>.rc"
    INPUT "${WINDOWS_RC_FILE}-configured.rc")
if (BUILD_CLI_WRAPPER AND META_PROJECT_TYPE STREQUAL "application")
    configure_file("${RC_CLI_TEMPLATE_FILE}" "${WINDOWS_CLI_RC_FILE}-configured.rc")
    file(
        GENERATE
        OUTPUT "${WINDOWS_CLI_RC_FILE}-$<CONFIG>.rc"
        INPUT "${WINDOWS_CLI_RC_FILE}-configured.rc")
endif ()

# add resource file to sources
list(APPEND RES_FILES "${WINDOWS_RC_FILE}-${CMAKE_BUILD_TYPE}.rc")
set_property(SOURCE "${WINDOWS_RC_FILE}-${CMAKE_BUILD_TYPE}.rc" PROPERTY GENERATED ON)
if (BUILD_CLI_WRAPPER AND META_PROJECT_TYPE STREQUAL "application")
    list(APPEND CLI_WRAPPER_RES_FILES "${WINDOWS_CLI_RC_FILE}-${CMAKE_BUILD_TYPE}.rc")
    set_property(SOURCE "${WINDOWS_CLI_RC_FILE}-${CMAKE_BUILD_TYPE}.rc" PROPERTY GENERATED ON)
endif ()

# configure resource compiler; use windres when compiling with mingw-w64
if (MINGW)
    set(CMAKE_RC_COMPILER_INIT windres)
    set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
endif ()
enable_language(RC)
