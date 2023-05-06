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
    # create a shared library which can be loaded from the Java-side, needs to be a module target to avoid
    # "QT_ANDROID_GENERATE_DEPLOYMENT_SETTINGS only works on Module targets" when using
    # `qt_android_generate_deployment_settings`.
    add_library(${META_TARGET_NAME} MODULE ${ALL_FILES})
    # set suffix to avoid "Cannot find application binary in build dir …/lib…_arm64-v8a.so." when using
    # `qt_android_add_apk_target`.
    if (ANDROID_ABI)
        set_target_properties(${META_TARGET_NAME} PROPERTIES SUFFIX "_${ANDROID_ABI}.so")
    endif ()
    set_target_properties(${META_TARGET_NAME} PROPERTIES QT_ANDROID_VERSION_NAME
                                                         "${META_VERSION_MAJOR}.${META_VERSION_MINOR}.${META_VERSION_PATCH}")
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
    PROPERTIES LINK_SEARCH_START_STATIC ${STATIC_LINKAGE}
               LINK_SEARCH_END_STATIC ${STATIC_LINKAGE}
               AUTOGEN_TARGET_DEPENDS "${AUTOGEN_DEPS}"
               QT_DEFAULT_PLUGINS "${META_QT_DEFAULT_PLUGINS}")
if (NOT ANDROID)
    set_target_properties(${META_TARGET_NAME} PROPERTIES C_VISIBILITY_PRESET hidden CXX_VISIBILITY_PRESET hidden)
    # note: Android *.so files need CXX visibility set to default (see qtbase commit
    # 29b17fa335388c9b93f70c29b2398cf2fee65785). Otherwise loading the app will fail with the error "dlsym failed: undefined
    # symbol: main".
endif ()
if (NOT META_CXX_STANDARD STREQUAL "any")
    set_target_properties(${META_TARGET_NAME} PROPERTIES CXX_STANDARD "${META_CXX_STANDARD}")
endif ()

# set properties for macOS bundle and generate icon for macOS bundle
if (GUI_TYPE STREQUAL "MACOSX_BUNDLE")
    set_target_properties(
        "${META_TARGET_NAME}"
        PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "${META_TARGET_NAME}"
                   MACOSX_BUNDLE_GUI_IDENTIFIER "${META_TARGET_NAME}"
                   MACOSX_BUNDLE_BUNDLE_VERSION "${META_APP_VERSION}"
                   MACOSX_BUNDLE_LONG_VERSION_STRING "${META_APP_VERSION}"
                   MACOSX_BUNDLE_SHORT_VERSION_STRING "${META_APP_VERSION}")
    if (MACOSX_ICON_PATH)
        target_sources(${META_TARGET_NAME} PRIVATE "${MACOSX_ICON_PATH}")
    elseif (PNG_ICON_PATH)
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

# create CLI-wrapper to be able to use CLI in Windows-termial without hacks
if (GUI_TYPE STREQUAL "WIN32")
    option(BUILD_CLI_WRAPPER "whether to build a CLI wrapper" ON)
endif ()
if (BUILD_CLI_WRAPPER)
    set(CLI_WRAPPER_TARGET_NAME "${META_TARGET_NAME}-cli")

    # add Boost::boost target which represents include directory for header-only deps and add Boost::filesystem as it is
    # needed by Boost.Process
    set(BOOST_ARGS 1.75 REQUIRED COMPONENTS filesystem)
    set(USE_PACKAGE_ARGS
        LIBRARIES_VARIABLE CLI_WRAPPER_TARGET_NAME_LIBS
        PACKAGES_VARIABLE CLI_WRAPPER_TARGET_NAME_PKGS)
    use_package(TARGET_NAME Boost::boost PACKAGE_NAME Boost PACKAGE_ARGS "${BOOST_ARGS}" ${USE_PACKAGE_ARGS})
    use_package(TARGET_NAME Boost::filesystem PACKAGE_NAME Boost PACKAGE_ARGS "${BOOST_ARGS}" ${USE_PACKAGE_ARGS})

    # find source file
    include(TemplateFinder)
    find_template_file_full_name("cli-wrapper.cpp" CPP_UTILITIES CLI_WRAPPER_SRC_FILE)

    # add and configure additional executable
    add_executable(${CLI_WRAPPER_TARGET_NAME} ${CLI_WRAPPER_RES_FILES} ${CLI_WRAPPER_SRC_FILE})
    target_link_libraries(${CLI_WRAPPER_TARGET_NAME} PRIVATE "${CLI_WRAPPER_TARGET_NAME_LIBS}")
    if (MINGW)
        # workaround https://github.com/boostorg/process/issues/96
        target_compile_definitions(${CLI_WRAPPER_TARGET_NAME} PRIVATE BOOST_USE_WINDOWS_H WIN32_LEAN_AND_MEAN)
    elseif (MSVC)
        # prevent "Please define _WIN32_WINNT or _WIN32_WINDOWS appropriately."
        target_compile_definitions(${CLI_WRAPPER_TARGET_NAME} PRIVATE _WIN32_WINNT=0x0601)
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
        if (CLI_WRAPPER_TARGET_NAME)
            install(TARGETS ${CLI_WRAPPER_TARGET_NAME} RUNTIME DESTINATION bin COMPONENT binary)
        endif ()
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
            RENAME "${NAMESPACE_PREFIX}${ICON_FILE_NAME}${META_CONFIG_SUFFIX}${ICON_FILE_EXT}"
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

include(AppUtilities)

set(TARGET_CONFIG_DONE YES)
