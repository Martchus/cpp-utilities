cmake_minimum_required(VERSION 3.3.0 FATAL_ERROR)

if (NOT BASIC_PROJECT_CONFIG_DONE)
    message(FATAL_ERROR "Before including the AppTarget module, the BasicConfig module must be included.")
endif ()
if (TARGET_CONFIG_DONE)
    message(FATAL_ERROR "Can not include AppTarget module when targets are already configured.")
endif ()

# check whether project type is set correctly
if (NOT "${META_PROJECT_TYPE}" STREQUAL "application")
    message(
        FATAL_ERROR
            "The AppTarget CMake module is intended to be used for building application projects only (and not for libraries)."
    )
endif ()

# define relevant files
set(ALL_FILES
    ${HEADER_FILES}
    ${SRC_FILES}
    ${GENERATED_DBUS_FILES}
    ${WIDGETS_FILES}
    ${QML_FILES}
    ${RES_FILES}
    ${WINDOWS_ICON_PATH})
if (NOT BUILTIN_TRANSLATIONS)
    list(APPEND ALL_FILES ${QM_FILES})
endif ()

# add custom libraries
append_user_defined_additional_libraries()

# add target for building the application
if (ANDROID)
    # create a shared library which can be loaded from the Java-side
    add_library(${META_TARGET_NAME} SHARED ${ALL_FILES})
else ()
    add_executable(${META_TARGET_NAME} ${GUI_TYPE} ${ALL_FILES})
endif ()
target_link_libraries(
    ${META_TARGET_NAME}
    PUBLIC ${META_ADDITIONAL_LINK_FLAGS} "${PUBLIC_LIBRARIES}"
    PRIVATE "${PRIVATE_LIBRARIES}")
target_include_directories(
    ${META_TARGET_NAME}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> ${PUBLIC_INCLUDE_DIRS}
    PRIVATE "${PRIVATE_INCLUDE_DIRS}")
target_compile_definitions(
    ${META_TARGET_NAME}
    PUBLIC "${META_PUBLIC_COMPILE_DEFINITIONS}"
    PRIVATE "${META_PRIVATE_COMPILE_DEFINITIONS}")
target_compile_options(
    ${META_TARGET_NAME}
    PUBLIC "${META_PUBLIC_COMPILE_OPTIONS}"
    PRIVATE "${META_PRIVATE_COMPILE_OPTIONS}")
set_target_properties(
    ${META_TARGET_NAME}
    PROPERTIES CXX_STANDARD "${META_CXX_STANDARD}"
               C_VISIBILITY_PRESET hidden
               CXX_VISIBILITY_PRESET hidden
               LINK_SEARCH_START_STATIC ${STATIC_LINKAGE}
               LINK_SEARCH_END_STATIC ${STATIC_LINKAGE}
               AUTOGEN_TARGET_DEPENDS "${AUTOGEN_DEPS}"
               QT_DEFAULT_PLUGINS "${META_QT_DEFAULT_PLUGINS}")

# set properties for macOS bundle and generate icon for macOS bundle
if (GUI_TYPE STREQUAL "MACOSX_BUNDLE")
    set_target_properties(
        "${META_TARGET_NAME}"
        PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "${META_TARGET_NAME}"
                   MACOSX_BUNDLE_GUI_IDENTIFIER "${META_TARGET_NAME}"
                   MACOSX_BUNDLE_BUNDLE_VERSION "${META_APP_VERSION}"
                   MACOSX_BUNDLE_LONG_VERSION_STRING "${META_APP_VERSION}"
                   MACOSX_BUNDLE_SHORT_VERSION_STRING "${META_APP_VERSION}")
    if (PNG_ICON_PATH)
        find_program(PNG2ICNS_BIN png2icns)
        if (PNG2ICNS_BIN)
            set(RESOURCES_DIR "${CMAKE_CURRENT_BINARY_DIR}/${META_TARGET_NAME}.app/Contents/Resources")
            set(MACOSX_ICON_PATH "${RESOURCES_DIR}/${META_PROJECT_NAME}.icns")
            add_custom_command(
                COMMENT "Generating icon for macOS bundle"
                OUTPUT "${MACOSX_ICON_PATH}"
                COMMAND "${CMAKE_COMMAND}" -E make_directory "${RESOURCES_DIR}"
                COMMAND ${PNG2ICNS_BIN} "${MACOSX_ICON_PATH}" "${PNG_ICON_PATH}"
                DEPENDS "${PNG_ICON_PATH}")
            message(STATUS "Generating macOS icon from \"${PNG_ICON_PATH}\" via ${PNG2ICNS_BIN}.")
            set_target_properties(${META_TARGET_NAME} PROPERTIES MACOSX_BUNDLE_ICON_FILE ${META_PROJECT_NAME}.icns)
            target_sources(${META_TARGET_NAME} PRIVATE "${MACOSX_ICON_PATH}")
        else ()
            message(STATUS "Unable to find png2icns, not creating a macOS bundle icon")
        endif ()
    endif ()
endif ()

# add install targets
if (NOT META_NO_INSTALL_TARGETS AND ENABLE_INSTALL_TARGETS)
    # add install target for binary
    if (APPLE)
        set(BUNDLE_INSTALL_DESTINATION
            "${CMAKE_INSTALL_BINDIR}"
            CACHE STRING "specifies the install destination for bundles")
        install(
            TARGETS ${META_TARGET_NAME}
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
            BUNDLE DESTINATION "${BUNDLE_INSTALL_DESTINATION}" COMPONENT binary)
    elseif (ANDROID)
        install(
            TARGETS ${META_TARGET_NAME}
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT binary
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}${SELECTED_LIB_SUFFIX}" COMPONENT binary
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}${SELECTED_LIB_SUFFIX}" COMPONENT binary)
    else ()
        install(TARGETS ${META_TARGET_NAME} RUNTIME DESTINATION bin COMPONENT binary)
    endif ()

    if (NOT TARGET install-binary)
        add_custom_target(install-binary COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=binary -P
                                                 "${CMAKE_BINARY_DIR}/cmake_install.cmake")
    endif ()
    add_dependencies(install-binary ${META_TARGET_NAME})

    # add mingw-w64 specific install target
    if (NOT TARGET install-mingw-w64)
        add_custom_target(install-mingw-w64)
        add_dependencies(install-mingw-w64 install-binary)
    endif ()
    add_dependencies(install-mingw-w64 ${META_TARGET_NAME})

    # add install target for desktop entries and icons
    foreach (DESKTOP_FILE ${DESKTOP_FILES})
        install(
            FILES "${DESKTOP_FILE}"
            DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications"
            COMPONENT desktop)
    endforeach ()

    foreach (ICON_FILE ${ICON_FILES})
        get_filename_component(ICON_FILE_NAME "${ICON_FILE}" NAME_WE)
        get_filename_component(ICON_FILE_EXT "${ICON_FILE}" EXT)
        install(
            FILES "${ICON_FILE}"
            RENAME "${ICON_FILE_NAME}${META_CONFIG_SUFFIX}${ICON_FILE_EXT}"
            DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps"
            COMPONENT desktop)
    endforeach ()
    if (NOT TARGET install-desktop)
        add_custom_target(install-desktop COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=desktop -P
                                                  "${CMAKE_BINARY_DIR}/cmake_install.cmake")
    endif ()
    add_dependencies(install-desktop ${META_TARGET_NAME})
    if (NOT TARGET install-appimage)
        add_custom_target(install-appimage COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=appimage -P
                                                   "${CMAKE_BINARY_DIR}/cmake_install.cmake")
    endif ()
    add_dependencies(install-appimage ${META_TARGET_NAME})

    # add install target for stripped binaries
    if (NOT TARGET install-binary-strip)
        add_custom_target(
            install-binary-strip COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_DO_STRIP=1 -DCMAKE_INSTALL_COMPONENT=binary -P
                                         "${CMAKE_BINARY_DIR}/cmake_install.cmake")
    endif ()
    add_dependencies(install-binary-strip ${META_TARGET_NAME})

    # add mingw-w64 specific install targets
    if (MINGW)
        if (NOT TARGET install-mingw-w64)
            add_custom_target(install-mingw-w64)
            add_dependencies(install-mingw-w64 install-binary)
        endif ()
        if (NOT TARGET install-mingw-w64-strip)
            add_custom_target(install-mingw-w64-strip)
            add_dependencies(install-mingw-w64-strip install-binary-strip)
        endif ()
        if (LOCALIZATION_TARGET)
            add_dependencies(install-mingw-w64 ${LOCALIZATION_TARGET})
            add_dependencies(install-mingw-w64-strip ${LOCALIZATION_TARGET})
        endif ()
    endif ()
endif ()

# find template for *.desktop and AppStream files
include(TemplateFinder)
find_template_file("desktop" CPP_UTILITIES APP_DESKTOP_TEMPLATE_FILE)
find_template_file("appdata.xml" CPP_UTILITIES APP_APPSTREAM_TEMPLATE_FILE)

# define generic function to add *.desktop files
include(CMakeParseArguments)
function (add_custom_desktop_file)
    # parse arguments
    set(ONE_VALUE_ARGS
        FILE_NAME
        DESKTOP_FILE_APP_NAME
        DESKTOP_FILE_GENERIC_NAME
        DESKTOP_FILE_DESCRIPTION
        DESKTOP_FILE_CATEGORIES
        DESKTOP_FILE_CMD
        DESKTOP_FILE_ICON
        DESKTOP_FILE_ADDITIONAL_ENTRIES)
    set(MULTI_VALUE_ARGS)
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

# define function to add *.desktop file and meta info from project meta data
function (add_desktop_file)
    # compose actions
    set(DESKTOP_FILE_ADDITIONAL_ENTRIES "")
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
        "${META_PROJECT_NAME}${META_CONFIG_SUFFIX}"
        DESKTOP_FILE_ADDITIONAL_ENTRIES
        "${DESKTOP_FILE_ADDITIONAL_ENTRIES}")

    # read body for appstream desktop file from resources
    set(META_APP_APPDATA_BODY_FILE "${CMAKE_CURRENT_SOURCE_DIR}/resources/body.appdata.xml")
    set(META_APP_APPDATA_SUBSTITUTED_BODY_FILE "${CMAKE_CURRENT_BINARY_DIR}/resources/${META_ID}.body.appdata.xml")
    if (EXISTS "${META_APP_APPDATA_BODY_FILE}")
        configure_file("${META_APP_APPDATA_BODY_FILE}" "${META_APP_APPDATA_SUBSTITUTED_BODY_FILE}" @ONLY)
        file(READ "${META_APP_APPDATA_SUBSTITUTED_BODY_FILE}" META_APP_APPDATA_BODY)
        # add indentation of two additional spaces
        string(REGEX REPLACE "\n([^$])" "\n  \\1" META_APP_APPDATA_BODY "${META_APP_APPDATA_BODY}")
    endif ()
    # create appstream desktop file from template
    configure_file("${APP_APPSTREAM_TEMPLATE_FILE}" "${CMAKE_CURRENT_BINARY_DIR}/resources/${META_ID}.appdata.xml" @ONLY)
    # add install for the appstream file
    install(
        FILES "${CMAKE_CURRENT_BINARY_DIR}/resources/${META_ID}.appdata.xml"
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/metainfo"
        COMPONENT appimage)
endfunction ()

set(TARGET_CONFIG_DONE YES)
