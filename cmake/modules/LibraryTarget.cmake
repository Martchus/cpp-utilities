# before including this module, BasicConfig must be included

# include for configure_package_config_file and write_basic_package_version_file
include(CMakePackageConfigHelpers)

# find template for CMake config file
include(TemplateFinder)
find_template_file("Config.cmake" CPP_UTILITIES CONFIG_TEMPLATE_FILE)

# determine library directory suffix
set(LIB_SUFFIX "" CACHE STRING "specifies the general suffix for the library directory")
set(SELECTED_LIB_SUFFIX "${LIB_SUFFIX}")
set(LIB_SUFFIX_32 "" CACHE STRING "specifies the suffix for the library directory to be used when building 32-bit library")
set(LIB_SUFFIX_64 "" CACHE STRING "specifies the suffix for the library directory to be used when building 64-bit library")
if(CMAKE_SIZEOF_VOID_P MATCHES "8" AND NOT ${LIB_SUFFIX_64} STREQUAL "")
    set(SELECTED_LIB_SUFFIX "${LIB_SUFFIX_64}")
elseif(CMAKE_SIZEOF_VOID_P MATCHES "4" AND NOT ${LIB_SUFFIX_32} STREQUAL "")
    set(SELECTED_LIB_SUFFIX "${LIB_SUFFIX_32}")
endif()

# set install destination for the CMake modules, config files and header files
set(HEADER_INSTALL_DESTINATION "${CMAKE_INSTALL_PREFIX}/include")
set(LIB_INSTALL_DESTINATION "${CMAKE_INSTALL_PREFIX}/lib${SELECTED_LIB_SUFFIX}")
set(CMAKE_MODULE_INSTALL_DESTINATION "${CMAKE_INSTALL_PREFIX}/share/${META_PROJECT_NAME}/cmake/modules")
set(CMAKE_CONFIG_INSTALL_DESTINATION "${CMAKE_INSTALL_PREFIX}/share/${META_PROJECT_NAME}/cmake")

# create the CMake config file from the template
configure_package_config_file(
    "${CONFIG_TEMPLATE_FILE}"
    "${CMAKE_CURRENT_BINARY_DIR}/${META_PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION
        "${CMAKE_CONFIG_INSTALL_DESTINATION}"
    PATH_VARS
        CMAKE_MODULE_INSTALL_DESTINATION
        CMAKE_CONFIG_INSTALL_DESTINATION
        HEADER_INSTALL_DESTINATION
        LIB_INSTALL_DESTINATION
)

# write the CMake version config file
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${META_PROJECT_NAME}ConfigVersion.cmake
    VERSION "${META_VERSION_MAJOR}.${META_VERSION_MINOR}.${META_VERSION_PATCH}"
    COMPATIBILITY SameMajorVersion
)

# remove library prefix when building with mingw-w64 (just for consistency with qmake)
if(MINGW)
    set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif(MINGW)

# set the windows extension to "dll", this is required by the mingw-w64 specific WindowsResources module
if(MINGW)
    set(WINDOWS_EXT "dll")
endif(MINGW)

# add target for building the library
add_library(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX} SHARED ${HEADER_FILES} ${SRC_FILES} ${WIDGETS_FILES} ${QML_FILES} ${RES_FILES} ${QM_FILES} ${WINDOWS_ICON_PATH})
target_link_libraries(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX} ${LIBRARIES})
set_target_properties(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX} PROPERTIES
    VERSION ${META_VERSION_MAJOR}.${META_VERSION_MINOR}.${META_VERSION_PATCH}
    SOVERSION ${META_VERSION_MAJOR}
    CXX_STANDARD 11
)

# add target for building a static version of the library
if(MINGW)
    set(BUILD_STATIC_LIBS "yes" CACHE STRING "specifies whether to build static libraries (enabled by default on mingw-w64 platform)")
else()
    set(BUILD_STATIC_LIBS "no" CACHE STRING "specifies whether to build static libraries (disabled by default on none-mingw-w64- platform)")
endif()
if(${BUILD_STATIC_LIBS} STREQUAL "yes")
    add_library(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_static STATIC ${HEADER_FILES} ${SRC_FILES} ${WIDGETS_FILES} ${QML_FILES} ${RES_FILES} ${QM_FILES} ${WINDOWS_ICON_PATH})
    # add target link libraries for the static lib also because otherwise Qt header files can not be located
    target_link_libraries(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_static ${LIBRARIES})
    set_target_properties(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_static PROPERTIES
        VERSION ${META_VERSION_MAJOR}.${META_VERSION_MINOR}.${META_VERSION_PATCH}
        SOVERSION ${META_VERSION_MAJOR}
        OUTPUT_NAME ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}
        CXX_STANDARD 11
    )
elseif(${BUILD_STATIC_LIBS} STREQUAL "no")
else()
    message(FATAL_ERROR "The specification whether to build static libs or not is invalid.")
endif(${BUILD_STATIC_LIBS} STREQUAL "yes")

# add install target for the CMake config files
install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${META_PROJECT_NAME}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${META_PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION
        "${CMAKE_CONFIG_INSTALL_DESTINATION}"
    COMPONENT
        cmake-config
)

if(NOT TARGET install-cmake-config)
    add_custom_target(install-cmake-config
        DEPENDS ${META_PROJECT_NAME}
        COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=cmake-config -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
    )
endif()

# add install target for dynamic libs
install(
    TARGETS ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}
    RUNTIME DESTINATION bin
    COMPONENT binary
    LIBRARY DESTINATION lib${SELECTED_LIB_SUFFIX}
    COMPONENT binary
    ARCHIVE DESTINATION lib${SELECTED_LIB_SUFFIX}
    COMPONENT binary
)

if(NOT TARGET install-binary)
    add_custom_target(install-binary
        DEPENDS ${META_PROJECT_NAME}
        COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=binary -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
    )
endif()

# add install for static libs when building with mingw-w64
if(${BUILD_STATIC_LIBS} STREQUAL "yes")
    install(
        TARGETS ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_static
        RUNTIME DESTINATION bin
        COMPONENT binary
        LIBRARY DESTINATION lib${SELECTED_LIB_SUFFIX}
        COMPONENT binary
        ARCHIVE DESTINATION lib${SELECTED_LIB_SUFFIX}
        COMPONENT binary
    )
endif(${BUILD_STATIC_LIBS} STREQUAL "yes")

# add install target for stripped libs
if(NOT TARGET install-binary-strip)
    add_custom_target(install-binary-strip
        DEPENDS ${META_PROJECT_NAME}
        COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_DO_STRIP=1 -DCMAKE_INSTALL_COMPONENT=binary -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
    )
endif()

# add install target for header files
foreach(HEADER_FILE ${HEADER_FILES} ${ADDITIONAL_HEADER_FILES})
    get_filename_component(HEADER_DIR "${HEADER_FILE}" DIRECTORY)
    install(
        FILES "${HEADER_FILE}"
        DESTINATION "include/${META_PROJECT_NAME}/${HEADER_DIR}"
        COMPONENT header
    )
endforeach()

if(NOT TARGET install-header)
    add_custom_target(install-header
        DEPENDS ${META_PROJECT_NAME}
        COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=header -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
    )
endif()

# add install target for CMake modules
foreach(CMAKE_MODULE_FILE ${CMAKE_MODULE_FILES})
    get_filename_component(CMAKE_MODULE_DIR ${CMAKE_MODULE_FILE} DIRECTORY)
    install(
        FILES ${CMAKE_MODULE_FILE}
        DESTINATION share/${META_PROJECT_NAME}/${CMAKE_MODULE_DIR}
        COMPONENT cmake-modules
    )
endforeach()

if(NOT TARGET install-cmake-modules)
    add_custom_target(install-cmake-modules
        DEPENDS ${META_PROJECT_NAME}
        COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=cmake-modules -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
    )
endif()

# add install target for CMake templates
foreach(CMAKE_TEMPLATE_FILE ${CMAKE_TEMPLATE_FILES})
    get_filename_component(CMAKE_TEMPLATE_DIR ${CMAKE_TEMPLATE_FILE} DIRECTORY)
    install(
        FILES ${CMAKE_TEMPLATE_FILE}
        DESTINATION share/${META_PROJECT_NAME}/${CMAKE_TEMPLATE_DIR}
        COMPONENT cmake-templates
    )
endforeach()

if(NOT TARGET install-cmake-templates)
    add_custom_target(install-cmake-templates
        DEPENDS ${META_PROJECT_NAME}
        COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=cmake-templates -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
    )
endif()

# add install target for all the cmake stuff
if(NOT TARGET install-cmake-stuff)
    add_custom_target(install-cmake-stuff
        DEPENDS install-cmake-config install-cmake-modules install-cmake-templates
    )
endif()

# add mingw-w64 specific install targets
if(NOT TARGET install-mingw-w64)
    add_custom_target(install-mingw-w64
        DEPENDS install-binary install-header install-cmake-stuff ${LOCALIZATION_TARGET}
    )
endif()

if(NOT TARGET install-mingw-w64-importlib-strip)
    add_custom_target(install-mingw-w64-importlib-strip
        DEPENDS install-binary-strip
        COMMAND "${CMAKE_FIND_ROOT_PATH}/bin/strip" --strip-unneeded "\$\{DESTDIR\}\$\{DESTDIR:+/\}${CMAKE_INSTALL_PREFIX}/lib/lib${META_PROJECT_NAME}.dll.a"
    )
endif()

if(NOT TARGET install-mingw-w64-staticlib-strip)
    add_custom_target(install-mingw-w64-staticlib-strip
        DEPENDS install-binary-strip
        COMMAND "${CMAKE_FIND_ROOT_PATH}/bin/strip" -g "\$\{DESTDIR\}\$\{DESTDIR:+/\}${CMAKE_INSTALL_PREFIX}/lib/lib${META_PROJECT_NAME}.a"
    )
endif()

if(NOT TARGET install-mingw-w64-strip)
    add_custom_target(install-mingw-w64-strip
        DEPENDS install-binary-strip install-mingw-w64-importlib-strip install-mingw-w64-staticlib-strip install-header install-cmake-stuff ${LOCALIZATION_TARGET}
    )
endif()
