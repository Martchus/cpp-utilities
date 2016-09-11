# before including this module, BasicConfig must be included

# check whether project type is set correctly
if((NOT "${META_PROJECT_TYPE}" STREQUAL "library") AND (NOT "${META_PROJECT_TYPE}" STREQUAL ""))
    message(FATAL_ERROR "The LibraryTarget CMake module is intended to be used for building library projects only (and not for applications).")
endif()

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
if(LIB_SUFFIX_64 AND CMAKE_SIZEOF_VOID_P MATCHES "8")
    set(SELECTED_LIB_SUFFIX "${LIB_SUFFIX_64}")
elseif(LIB_SUFFIX_32 AND CMAKE_SIZEOF_VOID_P MATCHES "4")
    set(SELECTED_LIB_SUFFIX "${LIB_SUFFIX_32}")
endif()

# set install destination for the CMake modules, config files and header files
set(HEADER_INSTALL_DESTINATION "${CMAKE_INSTALL_PREFIX}/include")
set(BIN_INSTALL_DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
set(LIB_INSTALL_DESTINATION "${CMAKE_INSTALL_PREFIX}/lib${SELECTED_LIB_SUFFIX}")
set(CMAKE_MODULE_INSTALL_DESTINATION "${CMAKE_INSTALL_PREFIX}/share/${META_PROJECT_NAME}/cmake/modules")
set(CMAKE_CONFIG_INSTALL_DESTINATION "${CMAKE_INSTALL_PREFIX}/share/${META_PROJECT_NAME}/cmake")


# remove library prefix when building with mingw-w64 (just for consistency with qmake)
if(MINGW)
    set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif(MINGW)

# set the windows extension to "dll", this is required by the mingw-w64 specific WindowsResources module
if(MINGW)
    set(WINDOWS_EXT "dll")
endif(MINGW)

# create global header and define build flags
if(NOT META_SHARED_LIB_COMPILE_DEFINITIONS)
    set(META_SHARED_LIB_COMPILE_DEFINITIONS ${META_COMPILE_DEFINITIONS})
endif()
if(NOT META_STATIC_LIB_COMPILE_DEFINITIONS)
    set(META_STATIC_LIB_COMPILE_DEFINITIONS ${META_COMPILE_DEFINITIONS} ${META_PROJECT_VARNAME_UPPER}_STATIC)
endif()

# add global library-specific header
include(TemplateFinder)
find_template_file("global.h" CPP_UTILITIES GLOBAL_H_TEMPLATE_FILE)
if("${META_PROJECT_NAME}" STREQUAL "c++utilities")
    set(GENERAL_GLOBAL_H_INCLUDE_PATH "\"./application/global.h\"")
else()
    set(GENERAL_GLOBAL_H_INCLUDE_PATH "<c++utilities/application/global.h>")
endif()
configure_file(
    "${GLOBAL_H_TEMPLATE_FILE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/global.h" # simply add this to source to ease inclusion
    NEWLINE_STYLE UNIX # since this goes to sources ensure consistency
)
list(APPEND HEADER_FILES global.h)

# add target for building the library
if(BUILD_SHARED_LIBS)
    # use correct linker flags and compile definitions (depend on linkage)
    if(STATIC_LIBRARY_LINKAGE)
        set(ACTUAL_ADDITIONAL_LINK_FLAGS ${ADDITIONAL_STATIC_LINK_FLAGS})
        set(ACTUAL_ADDITIONAL_COMPILE_DEFINITIONS ${ADDITIONAL_STATIC_COMPILE_DEFINITIONS})
    else()
        set(ACTUAL_ADDITIONAL_LINK_FLAGS ${ADDITIONAL_LINK_FLAGS})
        set(ACTUAL_ADDITIONAL_COMPILE_DEFINITIONS ${ADDITIONAL_COMPILE_DEFINITIONS})
    endif()
    # add library to be created, set libs to link against, set version and C++ standard
    add_library(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX} SHARED ${HEADER_FILES} ${SRC_FILES} ${WIDGETS_FILES} ${QML_FILES} ${RES_FILES} ${QM_FILES} ${WINDOWS_ICON_PATH})
    target_link_libraries(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX} ${ACTUAL_ADDITIONAL_LINK_FLAGS} ${LIBRARIES})
    set_target_properties(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX} PROPERTIES
        VERSION ${META_VERSION_MAJOR}.${META_VERSION_MINOR}.${META_VERSION_PATCH}
        SOVERSION ${META_VERSION_MAJOR}
        CXX_STANDARD 11
        COMPILE_DEFINITIONS "${ACTUAL_ADDITIONAL_COMPILE_DEFINITIONS}"
        LINK_SEARCH_START_STATIC ${STATIC_LINKAGE}
        LINK_SEARCH_END_STATIC ${STATIC_LINKAGE}
    )
endif()

# add target for building a static version of the library
if(BUILD_STATIC_LIBS)
    set(ACTUAL_ADDITIONAL_COMPILE_DEFINITIONS ${ADDITIONAL_STATIC_COMPILE_DEFINITIONS})
    list(APPEND ACTUAL_ADDITIONAL_COMPILE_DEFINITIONS ${META_STATIC_LIB_COMPILE_DEFINITIONS})
    add_library(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_static STATIC ${HEADER_FILES} ${SRC_FILES} ${WIDGETS_FILES} ${QML_FILES} ${RES_FILES} ${QM_FILES} ${WINDOWS_ICON_PATH})
    # add target link libraries for the static lib also because otherwise Qt header files can not be located
    target_link_libraries(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_static ${STATIC_LIBRARIES})
    set_target_properties(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_static PROPERTIES
        VERSION ${META_VERSION_MAJOR}.${META_VERSION_MINOR}.${META_VERSION_PATCH}
        SOVERSION ${META_VERSION_MAJOR}
        OUTPUT_NAME ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}
        CXX_STANDARD 11
        COMPILE_DEFINITIONS "${ACTUAL_ADDITIONAL_COMPILE_DEFINITIONS}"
    )
    set(META_STATIC_LIB_DEPENDS ${${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_static_LIB_DEPENDS}) # used in config file
endif()

# create the CMake package config file from template
configure_package_config_file(
    "${CONFIG_TEMPLATE_FILE}"
    "${CMAKE_CURRENT_BINARY_DIR}/${META_PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION
        "${CMAKE_CONFIG_INSTALL_DESTINATION}"
    PATH_VARS
        CMAKE_MODULE_INSTALL_DESTINATION
        CMAKE_CONFIG_INSTALL_DESTINATION
        HEADER_INSTALL_DESTINATION
        BIN_INSTALL_DESTINATION
        LIB_INSTALL_DESTINATION
)

# write the CMake version config file
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${META_PROJECT_NAME}ConfigVersion.cmake
    VERSION "${META_VERSION_MAJOR}.${META_VERSION_MINOR}.${META_VERSION_PATCH}"
    COMPATIBILITY SameMajorVersion
)

# create pkg-config file from template
# will (currently) not contain Libs.private if static libs haven't been built anyways
find_template_file("template.pc" CPP_UTILITIES PKGCONFIG_TEMPLATE_FILE)
configure_file(
    "${PKGCONFIG_TEMPLATE_FILE}"
    "${CMAKE_CURRENT_BINARY_DIR}/${META_PROJECT_NAME}.pc"
    @ONLY
)

# add install target for the CMake config files
install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${META_PROJECT_NAME}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${META_PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION
        "share/${META_PROJECT_NAME}/cmake"
    COMPONENT
        cmake-config
)
if(NOT TARGET install-cmake-config)
    add_custom_target(install-cmake-config
        DEPENDS ${META_PROJECT_NAME}
        COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=cmake-config -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
    )
endif()

# add install target for pkg-config file
install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/${META_PROJECT_NAME}.pc"
    DESTINATION "lib${SELECTED_LIB_SUFFIX}/lib"
    COMPONENT pkg-config
)
if(NOT TARGET install-pkg-config)
    add_custom_target(install-pkg-config
        DEPENDS ${META_PROJECT_NAME}
        COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=pkg-config -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
    )
endif()

# add install target for dynamic libs
if(BUILD_SHARED_LIBS)
    install(
        TARGETS ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}
        RUNTIME DESTINATION bin
        COMPONENT binary
        LIBRARY DESTINATION lib${SELECTED_LIB_SUFFIX}
        COMPONENT binary
        ARCHIVE DESTINATION lib${SELECTED_LIB_SUFFIX}
        COMPONENT binary
    )
endif()
if(NOT TARGET install-binary)
    add_custom_target(install-binary
        DEPENDS ${META_PROJECT_NAME}
        COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=binary -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
    )
endif()

# add install for static libs when building with mingw-w64
if(BUILD_STATIC_LIBS)
    install(
        TARGETS ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_static
        RUNTIME DESTINATION bin
        COMPONENT binary
        LIBRARY DESTINATION lib${SELECTED_LIB_SUFFIX}
        COMPONENT binary
        ARCHIVE DESTINATION lib${SELECTED_LIB_SUFFIX}
        COMPONENT binary
    )
endif()

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
set(ADDITIONAL_STRIP_TARGETS)
if(BUILD_SHARED_LIBS AND NOT TARGET install-mingw-w64-importlib-strip)
    add_custom_target(install-mingw-w64-importlib-strip
        DEPENDS install-binary-strip
        COMMAND "${CMAKE_FIND_ROOT_PATH}/bin/strip" --strip-unneeded "\$\{DESTDIR\}\$\{DESTDIR:+/\}${CMAKE_INSTALL_PREFIX}/lib/lib${META_PROJECT_NAME}.dll.a"
    )
    list(APPEND ADDITIONAL_STRIP_TARGETS install-mingw-w64-importlib-strip)
endif()
if(BUILD_STATIC_LIBS AND NOT TARGET install-mingw-w64-staticlib-strip)
    add_custom_target(install-mingw-w64-staticlib-strip
        DEPENDS install-binary-strip
        COMMAND "${CMAKE_FIND_ROOT_PATH}/bin/strip" -g "\$\{DESTDIR\}\$\{DESTDIR:+/\}${CMAKE_INSTALL_PREFIX}/lib/lib${META_PROJECT_NAME}.a"
    )
    list(APPEND ADDITIONAL_STRIP_TARGETS install-mingw-w64-staticlib-strip)
endif()
if(NOT TARGET install-mingw-w64-strip)
    add_custom_target(install-mingw-w64-strip
        DEPENDS install-binary-strip ${ADDITIONAL_STRIP_TARGETS} install-header install-cmake-stuff ${LOCALIZATION_TARGET}
    )
endif()
