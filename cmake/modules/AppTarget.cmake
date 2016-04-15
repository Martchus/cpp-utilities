# before including this module, BasicConfig must be included

# set the windows extension to "exe", this is required by the mingw-w64 specific WindowsResources module
if(MINGW)
    set(WINDOWS_EXT "exe")
endif(MINGW)

# add target for building the application
add_executable(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX} ${GUI_TYPE} ${HEADER_FILES} ${SRC_FILES} ${WIDGETS_FILES} ${QML_FILES} ${RES_FILES} ${QM_FILES} ${WINDOWS_ICON_PATH})
target_link_libraries(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX} ${LIBRARIES})
set_target_properties(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX} PROPERTIES
    CXX_STANDARD 11
)

# add install target for binary
install(TARGETS ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}
    RUNTIME DESTINATION bin
    COMPONENT binary
)

if(NOT TARGET install-binary)
    add_custom_target(install-binary
        DEPENDS ${META_PROJECT_NAME}
        COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=binary -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
    )
endif()

# add install target for localization
if(NOT TARGET install-mingw-w64)
    add_custom_target(install-mingw-w64
        DEPENDS install-binary ${LOCALIZATION_TARGET}
    )
endif()

# add install target for desktop entries and icons
foreach(DESKTOP_FILE ${DESKTOP_FILES})
    install(
        FILES "${DESKTOP_FILE}"
        DESTINATION "share/applications"
        COMPONENT desktop
    )
endforeach()

foreach(ICON_FILE ${ICON_FILES})
    install(
        FILES "${ICON_FILE}"
        DESTINATION "share/icons/hicolor/scalable/apps"
        COMPONENT desktop
    )
endforeach()

if(NOT TARGET install-desktop)
    add_custom_target(install-desktop
        DEPENDS ${META_PROJECT_NAME}
        COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=desktop -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
    )
endif()

# add install target for stripped binaries
if(NOT TARGET install-binary-strip)
    add_custom_target(install-binary-strip
        DEPENDS ${META_PROJECT_NAME}
        COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_DO_STRIP=1 -DCMAKE_INSTALL_COMPONENT=binary -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
    )
endif()

# add mingw-w64 specific install target
if(NOT TARGET install-mingw-w64-strip)
    add_custom_target(install-mingw-w64-strip
        DEPENDS install-binary-strip ${LOCALIZATION_TARGET}
    )
endif()

# find template for *.desktop files
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/desktop.in")
    # check own source directory
    set(APP_DESKTOP_TEMPLATE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/desktop.in")
    message(STATUS "Using template for *.desktop file from own source directory.")
elseif(EXISTS "${CPP_UTILITIES_SOURCE_DIR}/cmake/templates/desktop.in")
    # check sources of c++utilities
    set(APP_DESKTOP_TEMPLATE_FILE "${CPP_UTILITIES_SOURCE_DIR}/cmake/templates/desktop.in")
    message(STATUS "Using template for *.desktop file from c++utilities source directory.")
elseif(EXISTS "${CMAKE_INSTALL_PREFIX}/share/c++utilities/cmake/templates/desktop.in")
    # check installed version of c++utilities
    set(APP_DESKTOP_TEMPLATE_FILE "${CMAKE_INSTALL_PREFIX}/share/c++utilities/cmake/templates/desktop.in")
    message(STATUS "Using template for *.desktop file from c++utilities installation.")
else()
    message(FATAL_ERROR "Template for *.desktop file can not be located.")
endif()

# function to add *.desktop files
function(add_custom_desktop_file
        FILE_NAME
        DESKTOP_FILE_APP_NAME
        DESKTOP_FILE_DESCRIPTION
        DESKTOP_FILE_CATEGORIES
        DESKTOP_FILE_CMD
        DESKTOP_FILE_ICON)
    # create desktop file from template
    configure_file(
        "${APP_DESKTOP_TEMPLATE_FILE}"
        "${CMAKE_CURRENT_BINARY_DIR}/resources/${FILE_NAME}.desktop"
    )
    # add install for the desktop file
    install(
        FILES "${CMAKE_CURRENT_BINARY_DIR}/resources/${FILE_NAME}.desktop"
        DESTINATION "share/applications"
        COMPONENT desktop
    )
endfunction()

# convenience function to add *.desktop file from project meta data
function(add_desktop_file)
    add_custom_desktop_file(
        "${META_PROJECT_NAME}"
        "${META_APP_NAME}"
        "${META_APP_DESCRIPTION}"
        "${META_APP_CATEGORIES}"
        "${META_PROJECT_NAME}"
        "${META_PROJECT_NAME}"
    )
endfunction()
