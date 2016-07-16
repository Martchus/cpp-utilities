# generates and adds a Windows rc file for the application/library
# also attaches the application icon if ffmpeg is available
# does nothing if not building with mingw-w64

# before including this module, LibraryConfig/ApplicationConfig must be included

if(MINGW)
    # find rc template
    include(TemplateFinder)
    find_template_file("windows.rc" CPP_UTILITIES RC_TEMPLATE_FILE)

    # create Windows icon from png with ffmpeg if available
    set(WINDOWS_ICON_PATH "")
    set(WINDOWS_ICON_RC_ENTRY "")
    find_program(FFMPEG_BIN ffmpeg avconv)
    if(FFMPEG_BIN)
        if(NOT "${PNG_ICON_PATH}")
            set(PNG_ICON_PATH "${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/hicolor/128x128/apps/${META_PROJECT_NAME}.png")
        endif()
        if(EXISTS "${PNG_ICON_PATH}")
            set(WINDOWS_ICON_PATH "${CMAKE_CURRENT_BINARY_DIR}/${META_PROJECT_NAME}.ico")
            set(WINDOWS_ICON_RC_ENTRY "IDI_ICON1   ICON    DISCARDABLE \"${WINDOWS_ICON_PATH}\"")
            add_custom_command(
                OUTPUT "${WINDOWS_ICON_PATH}"
                COMMAND ${FFMPEG_BIN} -y -i "${PNG_ICON_PATH}" -vf crop=iw-20:ih-20:10:10,scale=64:64 "${WINDOWS_ICON_PATH}"
            )
        endif()
    endif(FFMPEG_BIN)

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
endif(MINGW)
