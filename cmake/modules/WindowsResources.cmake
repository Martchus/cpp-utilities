# generates and adds a Windows rc file for the application/library
# also attaches the application icon if ffmpeg is available
# does nothing if not building with mingw-w64

# before including this module, LibraryConfig/ApplicationConfig must be included

if(MINGW)
    # find rc template
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/windows.rc.in")
        # check own source directory
        set(RC_TEMPLATE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/windows.rc.in")
    elseif(EXISTS "${CPP_UTILITIES_SOURCE_DIR}/cmake/templates/windows.rc.in")
        # check sources of c++utilities
        set(RC_TEMPLATE_FILE "${CPP_UTILITIES_SOURCE_DIR}/cmake/templates/windows.rc.in")
    elseif(EXISTS "${CMAKE_INSTALL_PREFIX}/share/c++utilities/cmake/templates/windows.rc.in")
        # check installed version of c++utilities
        set(RC_TEMPLATE_FILE "${CMAKE_INSTALL_PREFIX}/share/c++utilities/cmake/templates/windows.rc.in")
    else()
        message(FATAL_ERROR "Template for Windows *.rc file can not be located.")
    endif()

    # create Windows icon from png with ffmpeg if available
    set(WINDOWS_ICON_PATH "")
    set(WINDOWS_ICON_RC_ENTRY "")
    find_program(FFMPEG_BIN ffmpeg avconv)
    if(FFMPEG_BIN)
        set(PNG_ICON_PATH "${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/hicolor/128x128/apps/${META_PROJECT_NAME}.png")
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
    set(RES_FILES "${CMAKE_CURRENT_BINARY_DIR}/resources/windows.rc")
    set(CMAKE_RC_COMPILER_INIT windres)
    set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
    enable_language(RC)
endif(MINGW)
