cmake_minimum_required(VERSION 3.17.0 FATAL_ERROR)

# prevent multiple inclusion
if (DEFINED APPLICATION_UTILITIES_LOADED)
    return()
endif ()
set(APPLICATION_UTILITIES_LOADED YES)

# find template for *.desktop and AppStream files
include(TemplateFinder)
find_template_file("desktop" CPP_UTILITIES APP_DESKTOP_TEMPLATE_FILE)
find_template_file("appdata.xml" CPP_UTILITIES APP_APPSTREAM_TEMPLATE_FILE)

# define generic function to add *.desktop files
include(CMakeParseArguments)
function (add_custom_desktop_file)
    # skip if not building with GUI support
    if (NOT META_DESKTOP_FILE_FOR_CLI AND ((NOT DEFINED META_GUI_OPTIONAL OR META_GUI_OPTIONAL) AND NOT (WIDGETS_GUI
                                                                                                         OR QUICK_GUI)))
        return()
    endif ()

    # parse arguments
    set(ONE_VALUE_ARGS FILE_NAME DESKTOP_FILE_APP_NAME DESKTOP_FILE_GENERIC_NAME DESKTOP_FILE_DESCRIPTION DESKTOP_FILE_CMD
                       DESKTOP_FILE_ICON)
    set(MULTI_VALUE_ARGS DESKTOP_FILE_CATEGORIES DESKTOP_FILE_ADDITIONAL_ENTRIES)
    set(OPTIONAL_ARGS)
    cmake_parse_arguments(ARGS "${OPTIONAL_ARGS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" ${ARGN})
    if (NOT ARGS_FILE_NAME
        OR NOT ARGS_DESKTOP_FILE_APP_NAME
        OR NOT ARGS_DESKTOP_FILE_CMD)
        message(FATAL_ERROR "Not all mandatory arguments specified.")
    endif ()
    if (NOT ARGS_DESKTOP_FILE_GENERIC_NAME)
        set(ARGS_DESKTOP_FILE_GENERIC_NAME "${ARGS_DESKTOP_FILE_APP_NAME}")
    endif ()

    # create desktop file from template
    configure_file("${APP_DESKTOP_TEMPLATE_FILE}" "${CMAKE_CURRENT_BINARY_DIR}/resources/${ARGS_FILE_NAME}.desktop")
    # add install for the desktop file
    install(
        FILES "${CMAKE_CURRENT_BINARY_DIR}/resources/${ARGS_FILE_NAME}.desktop"
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications"
        COMPONENT desktop)
endfunction ()

# define function to add appstream file
function (add_appstream_file)
    # read body for appstream desktop file from resources
    set(META_APP_APPDATA_BODY_FILE "${CMAKE_CURRENT_SOURCE_DIR}/resources/body.appdata.xml")
    set(META_APP_APPDATA_SUBSTITUTED_BODY_FILE "${CMAKE_CURRENT_BINARY_DIR}/resources/${META_ID}.body.appdata.xml")
    if (EXISTS "${META_APP_APPDATA_BODY_FILE}")
        configure_file("${META_APP_APPDATA_BODY_FILE}" "${META_APP_APPDATA_SUBSTITUTED_BODY_FILE}" @ONLY)
        file(READ "${META_APP_APPDATA_SUBSTITUTED_BODY_FILE}" META_APP_APPDATA_BODY)
        # add indentation of two additional spaces
        string(REGEX REPLACE "\n([^$])" "\n  \\1" META_APP_APPDATA_BODY "${META_APP_APPDATA_BODY}")
    endif ()

    # skip if not all required meta-data is present
    if (NOT META_RELEASE_DATE)
        message(STATUS "The variable META_RELEASE_DATE is not set. Not creating an AppStream file.")
        return()
    endif ()

    # create appstream desktop file from template
    set(APPSTREAM_FILE "${CMAKE_CURRENT_BINARY_DIR}/resources/${META_ID}.appdata.xml")
    configure_file("${APP_APPSTREAM_TEMPLATE_FILE}" "${APPSTREAM_FILE}" @ONLY)

    # add install for the appstream file
    install(
        FILES "${APPSTREAM_FILE}"
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/metainfo"
        COMPONENT appimage)

    # add test
    set(APPSTREAM_TESTS_ENABLED_DEFAULT OFF)
    find_program(APPSTREAMCLI_BIN "appstreamcli")
    if (ENABLE_DEVEL_DEFAULTS AND APPSTREAMCLI_BIN)
        set(APPSTREAM_TESTS_ENABLED_DEFAULT ON)
    endif ()
    option(APPSTREAM_TESTS_ENABLED "enables tests for checking whether AppStream files" "${APPSTREAM_TESTS_ENABLED_DEFAULT}")
    if (APPSTREAM_TESTS_ENABLED)
        if (NOT APPSTREAMCLI_BIN)
            message(FATAL_ERROR "Unable to validate appstreamcli files; appstreamcli not found")
        else ()
            add_test(NAME "${META_TARGET_NAME}_appstream_validation" COMMAND "${APPSTREAMCLI_BIN}" validate
                                                                             "${APPSTREAM_FILE}")
        endif ()
    endif ()
endfunction ()

# define function to add *.desktop file and meta info from project meta data
function (add_desktop_file)
    # skip if not building with GUI support
    if (NOT META_DESKTOP_FILE_FOR_CLI AND ((NOT DEFINED META_GUI_OPTIONAL OR META_GUI_OPTIONAL) AND NOT (WIDGETS_GUI
                                                                                                         OR QUICK_GUI)))
        return()
    endif ()

    # compose actions
    foreach (ACTION_VAR ${META_APP_ACTIONS})
        list(GET META_APP_ACTION_${ACTION_VAR} 0 ACTION_ID)
        list(GET META_APP_ACTION_${ACTION_VAR} 1 ACTION_NAME)
        list(GET META_APP_ACTION_${ACTION_VAR} 2 ACTION_ARGS)
        set(DESKTOP_FILE_ADDITIONAL_ENTRIES
            "${DESKTOP_FILE_ADDITIONAL_ENTRIES}\n[Desktop Action ${ACTION_ID}]\nName=${ACTION_NAME}\nExec=${META_TARGET_NAME} ${ACTION_ARGS}"
        )
    endforeach ()

    # create desktop file
    add_custom_desktop_file(
        FILE_NAME
        "${META_ID}"
        DESKTOP_FILE_APP_NAME
        "${META_APP_NAME}"
        DESKTOP_FILE_GENERIC_NAME
        "${META_GENERIC_NAME}"
        DESKTOP_FILE_DESCRIPTION
        "${META_APP_DESCRIPTION}"
        DESKTOP_FILE_CATEGORIES
        "${META_APP_CATEGORIES}"
        DESKTOP_FILE_CMD
        "${META_TARGET_NAME}"
        DESKTOP_FILE_ICON
        "${NAMESPACE_PREFIX}${META_PROJECT_NAME}${META_CONFIG_SUFFIX}"
        DESKTOP_FILE_ADDITIONAL_ENTRIES
        "${DESKTOP_FILE_ADDITIONAL_ENTRIES}")

    add_appstream_file()
endfunction ()
