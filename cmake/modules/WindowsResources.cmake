cmake_minimum_required(VERSION 3.3.0 FATAL_ERROR)

# generates and adds a Windows rc file for the application/library also attaches the application icon if ffmpeg is available
# does nothing if not building with mingw-w64

if (NOT BASIC_PROJECT_CONFIG_DONE)
    message(
        FATAL_ERROR
            "Before including the WindowsResources module, the LibraryConfig/ApplicationConfig module must be included.")
endif ()

option(WINDOWS_RESOURCES_ENABLED "controls whether Windows resources are enabled" ON)
option(WINDOWS_ICON_ENABLED "controls whether Windows icon is enabled" ON)

if (NOT MINGW OR NOT WINDOWS_RESOURCES_ENABLED)
    return()
endif ()

# find rc template, define path of output rc file
include(TemplateFinder)
find_template_file("windows.rc" CPP_UTILITIES RC_TEMPLATE_FILE)
set(WINDOWS_RC_FILE_CFG "${CMAKE_CURRENT_BINARY_DIR}/resources/windows.rc.configured")
set(WINDOWS_RC_FILE "${CMAKE_CURRENT_BINARY_DIR}/resources/windows")

# create Windows icon from png with ffmpeg if available
unset(WINDOWS_ICON_PATH)
unset(WINDOWS_ICON_RC_ENTRY)
if (WINDOWS_ICON_ENABLED AND PNG_ICON_PATH)
    find_program(FFMPEG_BIN ffmpeg avconv)
    if (FFMPEG_BIN)
        set(WINDOWS_ICON_PATH "${CMAKE_CURRENT_BINARY_DIR}/resources/${META_PROJECT_NAME}.ico")
        set(WINDOWS_ICON_RC_ENTRY "IDI_ICON1   ICON    DISCARDABLE \"${WINDOWS_ICON_PATH}\"")
        add_custom_command(
            COMMENT "Generating icon for Windows executable"
            OUTPUT "${WINDOWS_ICON_PATH}"
            COMMAND ${FFMPEG_BIN} -y -i "${PNG_ICON_PATH}" "${WINDOWS_ICON_PATH}"
            DEPENDS "${PNG_ICON_PATH}")
        set_source_files_properties("${WINDOWS_RC_FILE}" PROPERTIES OBJECT_DEPENDS "${WINDOWS_ICON_PATH}")
        message(STATUS "Generating Windows icon from \"${PNG_ICON_PATH}\" via ${FFMPEG_BIN}.")
    else ()
        message(STATUS "Unable to find ffmpeg, not creating a Windows icon")
    endif ()
endif ()

# create Windows rc file from template
configure_file("${RC_TEMPLATE_FILE}" "${WINDOWS_RC_FILE}-configured.rc")
file(
    GENERATE
    OUTPUT "${WINDOWS_RC_FILE}-$<CONFIG>.rc"
    INPUT "${WINDOWS_RC_FILE}-configured.rc")

# set windres as resource compiler
list(APPEND RES_FILES "${WINDOWS_RC_FILE}-${CMAKE_BUILD_TYPE}.rc")
set_property(
    SOURCE "${WINDOWS_RC_FILE}-${CMAKE_BUILD_TYPE}.rc"
    APPEND
    PROPERTY GENERATED ON)
set(CMAKE_RC_COMPILER_INIT windres)
set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
enable_language(RC)
