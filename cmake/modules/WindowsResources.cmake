# generates and adds a Windows rc file for the application/library
# also attaches the application icon if ffmpeg is available
# does nothing if not building with mingw-w64

if(NOT BASIC_PROJECT_CONFIG_DONE)
    message(FATAL_ERROR "Before including the WindowsResources module, the LibraryConfig/ApplicationConfig module must be included.")
endif()

option(WINDOWS_RESOURCES_ENABLED "controls whether Windows resources are enabled" ON)
option(WINDOWS_ICON_ENABLED "controls whether Windows icon is enabled" ON)

if(NOT MINGW OR NOT WINDOWS_RESOURCES_ENABLED)
    return()
endif()

# find rc template
include(TemplateFinder)
find_template_file("windows.rc" CPP_UTILITIES RC_TEMPLATE_FILE)

# create Windows icon from png with ffmpeg if available
unset(WINDOWS_ICON_PATH)
unset(WINDOWS_ICON_RC_ENTRY)
if(WINDOWS_ICON_ENABLED)
    find_program(FFMPEG_BIN ffmpeg avconv)
    if(FFMPEG_BIN)
        if(NOT PNG_ICON_PATH)
            set(PNG_ICON_PATH "${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/hicolor/128x128/apps/${META_PROJECT_NAME}.png")
            set(USING_DEFAULT_PNG_ICON_PATH true)
        endif()
        if(PNG_ICON_NO_CROP)
            unset(PNG_ICON_CROP)
        elseif(NOT PNG_ICON_CROP)
            # default cropping
            set(PNG_ICON_CROP "iw-20:ih-20:10:10")
        endif()
        if(EXISTS "${PNG_ICON_PATH}")
            set(WINDOWS_ICON_PATH "${CMAKE_CURRENT_BINARY_DIR}/resources/${META_PROJECT_NAME}.ico")
            set(WINDOWS_ICON_RC_ENTRY "IDI_ICON1   ICON    DISCARDABLE \"${WINDOWS_ICON_PATH}\"")
            add_custom_command(
                OUTPUT "${WINDOWS_ICON_PATH}"
                COMMAND ${FFMPEG_BIN} -y -i "${PNG_ICON_PATH}" -vf crop=${PNG_ICON_CROP},scale=64:64 "${WINDOWS_ICON_PATH}"
            )
            message(STATUS "Generating Windows icon from \"${PNG_ICON_PATH}\" via ${FFMPEG_BIN}.")
        elseif(NOT USING_DEFAULT_PNG_ICON_PATH)
            message(FATAL_ERROR "The specified PNG_ICON_PATH \"${PNG_ICON_PATH}\" is invalid.")
        endif()
        unset(USING_DEFAULT_PNG_ICON_PATH)
    endif()
endif()

# create Windows rc file from template
configure_file(
    "${RC_TEMPLATE_FILE}"
    "${CMAKE_CURRENT_BINARY_DIR}/resources/windows.rc"
)
# set windres as resource compiler
list(APPEND RES_FILES "${CMAKE_CURRENT_BINARY_DIR}/resources/windows.rc")
set(CMAKE_RC_COMPILER_INIT windres)
set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
enable_language(RC)
