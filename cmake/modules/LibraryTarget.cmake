cmake_minimum_required(VERSION 3.3.0 FATAL_ERROR)

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

# set compile definitions
if(NOT META_PUBLIC_SHARED_LIB_COMPILE_DEFINITIONS)
    set(META_PUBLIC_SHARED_LIB_COMPILE_DEFINITIONS ${META_PUBLIC_COMPILE_DEFINITIONS} ${META_ADDITIONAL_PUBLIC_SHARED_COMPILE_DEFINITIONS})
endif()
if(NOT META_PRIVATE_SHARED_LIB_COMPILE_DEFINITIONS)
    set(META_PRIVATE_SHARED_LIB_COMPILE_DEFINITIONS ${META_PRIVATE_COMPILE_DEFINITIONS} ${META_ADDITIONAL_PRIVATE_SHARED_COMPILE_DEFINITIONS})
endif()
if(NOT META_PUBLIC_STATIC_LIB_COMPILE_DEFINITIONS)
    set(META_PUBLIC_STATIC_LIB_COMPILE_DEFINITIONS ${META_PUBLIC_COMPILE_DEFINITIONS} ${META_PROJECT_VARNAME_UPPER}_STATIC ${META_ADDITIONAL_PUBLIC_STATIC_COMPILE_DEFINITIONS})
endif()
if(NOT META_PRIVATE_STATIC_LIB_COMPILE_DEFINITIONS)
    set(META_PRIVATE_STATIC_LIB_COMPILE_DEFINITIONS ${META_PRIVATE_COMPILE_DEFINITIONS} ${META_ADDITIONAL_PRIVATE_STATIC_COMPILE_DEFINITIONS})
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

# determine SOVERSION
if(NOT META_SOVERSION)
    if(META_VERSION_EXACT_SONAME)
        set(META_SOVERSION "${META_VERSION_MAJOR}.${META_VERSION_MINOR}.${META_VERSION_PATCH}")
    else()
        set(META_SOVERSION "${META_VERSION_MAJOR}")
    endif()
endif()

# add target for building the library
if(BUILD_SHARED_LIBS)
    if(STATIC_LIBRARY_LINKAGE)
        set(ACTUAL_ADDITIONAL_LINK_FLAGS ${META_ADDITIONAL_STATIC_LINK_FLAGS})
    else()
        set(ACTUAL_ADDITIONAL_LINK_FLAGS ${META_ADDITIONAL_SHARED_LINK_FLAGS})
    endif()
    # add library to be created, set libs to link against, set version and C++ standard
    add_library(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX} SHARED ${HEADER_FILES} ${SRC_FILES} ${WIDGETS_FILES} ${QML_FILES} ${RES_FILES} ${QM_FILES} ${WINDOWS_ICON_PATH})
    target_link_libraries(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}
        PUBLIC ${ACTUAL_ADDITIONAL_LINK_FLAGS} "${PUBLIC_LIBRARIES}"
        PRIVATE "${PRIVATE_LIBRARIES}"
    )
    target_compile_definitions(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}
        PUBLIC "${META_PUBLIC_SHARED_LIB_COMPILE_DEFINITIONS}"
        PRIVATE "${META_PRIVATE_SHARED_LIB_COMPILE_DEFINITIONS}"
    )
    set_target_properties(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX} PROPERTIES
        VERSION "${META_VERSION_MAJOR}.${META_VERSION_MINOR}.${META_VERSION_PATCH}"
        SOVERSION "${META_SOVERSION}"
        CXX_STANDARD 11
        LINK_SEARCH_START_STATIC ${STATIC_LINKAGE}
        LINK_SEARCH_END_STATIC ${STATIC_LINKAGE}
    )
endif()

# add target for building a static version of the library
if(BUILD_STATIC_LIBS)
    add_library(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_static STATIC ${HEADER_FILES} ${SRC_FILES} ${WIDGETS_FILES} ${QML_FILES} ${RES_FILES} ${QM_FILES} ${WINDOWS_ICON_PATH})
    # add target link libraries for the static lib also because otherwise Qt header files can not be located
    target_link_libraries(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_static
        PUBLIC "${PUBLIC_STATIC_LIBRARIES}"
        PRIVATE "${PRIVATE_STATIC_LIBRARIES}"
    )
    target_compile_definitions(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_static
        PUBLIC "${META_PUBLIC_STATIC_LIB_COMPILE_DEFINITIONS}"
        PRIVATE "${META_PRIVATE_STATIC_LIB_COMPILE_DEFINITIONS}"
    )
    set_target_properties(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_static PROPERTIES
        VERSION "${META_VERSION_MAJOR}.${META_VERSION_MINOR}.${META_VERSION_PATCH}"
        SOVERSION "${META_SOVERSION}"
        OUTPUT_NAME "${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}"
        CXX_STANDARD 11
    )
    foreach(DEPENDENCY ${${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_static_LIB_DEPENDS})
        if(NOT ${DEPENDENCY} IN_LIST META_PUBLIC_STATIC_LIB_DEPENDS)
            list(APPEND META_PRIVATE_STATIC_LIB_DEPENDS ${DEPENDENCY})
        endif()
    endforeach()
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
find_template_file("template.pc" CPP_UTILITIES PKGCONFIG_TEMPLATE_FILE)
macro(depends_for_pc LIB_TYPE DEPENDS OUTPUT_VAR_PKGS OUTPUT_VAR_LIBS)
    unset(${OUTPUT_VAR_PKGS})
    unset(${OUTPUT_VAR_LIBS})
    foreach(DEPENDENCY ${${DEPENDS}})
        if("${DEPENDENCY}" STREQUAL "general")
            continue()
        endif()
        string(REPLACE "::" "_" DEPENDENCY_VARNAME "${DEPENDENCY}")
        message(STATUS "PC_PKG_${LIB_TYPE}_${DEPENDENCY_VARNAME}: ${PC_PKG_${LIB_TYPE}_${DEPENDENCY_VARNAME}}")
        if(PC_PKG_${LIB_TYPE}_${DEPENDENCY_VARNAME})
            set(${OUTPUT_VAR_PKGS} "${${OUTPUT_VAR_PKGS}} ${PC_PKG_${LIB_TYPE}_${DEPENDENCY_VARNAME}}")
        else()
            set(${OUTPUT_VAR_LIBS} "${${OUTPUT_VAR_LIBS}} ${DEPENDENCY}")
        endif()
    endforeach()
endmacro()
macro(comple_defs_for_pc LIB_TYPE)
    foreach(COMPILE_DEFINITION ${META_PUBLIC_${LIB_TYPE}_LIB_COMPILE_DEFINITIONS})
        set(META_COMPILE_DEFINITIONS_FOR_PC "${META_COMPILE_DEFINITIONS_FOR_PC} -D${COMPILE_DEFINITION}")
    endforeach()
endmacro()
unset(PC_FILES)
if(BUILD_SHARED_LIBS)
    set(META_PROJECT_NAME_FOR_PC "${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}")
    depends_for_pc(SHARED META_PUBLIC_SHARED_LIB_DEPENDS META_PUBLIC_PC_PKGS META_PUBLIC_LIB_DEPENDS_FOR_PC)
    comple_defs_for_pc(SHARED)
    configure_file(
        "${PKGCONFIG_TEMPLATE_FILE}"
        "${CMAKE_CURRENT_BINARY_DIR}/${META_PROJECT_NAME_FOR_PC}.pc"
        @ONLY
    )
    list(APPEND PC_FILES ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}.pc)
endif()
if(BUILD_STATIC_LIBS)
    set(META_PROJECT_NAME_FOR_PC "${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_static")
    depends_for_pc(STATIC META_PUBLIC_STATIC_LIB_DEPENDS META_PUBLIC_PC_PKGS META_PUBLIC_LIB_DEPENDS_FOR_PC)
    depends_for_pc(STATIC META_PRIVATE_STATIC_LIB_DEPENDS META_PRIVATE_PC_PKGS META_PRIVATE_LIB_DEPENDS_FOR_PC)
    comple_defs_for_pc(STATIC)
    configure_file(
        "${PKGCONFIG_TEMPLATE_FILE}"
        "${CMAKE_CURRENT_BINARY_DIR}/${META_PROJECT_NAME_FOR_PC}.pc"
        @ONLY
    )
    list(APPEND PC_FILES ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_static.pc)
endif()

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
        COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=cmake-config -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
    )
endif()

# add install target for pkg-config file
if(PC_FILES)
    install(
        FILES ${PC_FILES}
        DESTINATION "lib${SELECTED_LIB_SUFFIX}/pkgconfig"
        COMPONENT pkg-config
    )
endif()
if(NOT TARGET install-pkg-config)
    add_custom_target(install-pkg-config
        COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=pkg-config -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
    )
endif()

# add install target for dynamic libs
if(BUILD_SHARED_LIBS)
    install(
        TARGETS ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}
        EXPORT ${META_PROJECT_NAME}SharedTargets
        RUNTIME DESTINATION bin
        COMPONENT binary
        LIBRARY DESTINATION lib${SELECTED_LIB_SUFFIX}
        COMPONENT binary
        ARCHIVE DESTINATION lib${SELECTED_LIB_SUFFIX}
        COMPONENT binary
    )
    install(EXPORT ${META_PROJECT_NAME}SharedTargets
        DESTINATION "share/${META_PROJECT_NAME}/cmake"
        EXPORT_LINK_INTERFACE_LIBRARIES
        COMPONENT cmake-config
    )
endif()
if(NOT TARGET install-binary)
    add_custom_target(install-binary
        COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=binary -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
    )
endif()

# add install for static libs when building with mingw-w64
if(BUILD_STATIC_LIBS)
    install(
        TARGETS ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_static
        EXPORT ${META_PROJECT_NAME}StaticTargets
        RUNTIME DESTINATION bin
        COMPONENT binary
        LIBRARY DESTINATION lib${SELECTED_LIB_SUFFIX}
        COMPONENT binary
        ARCHIVE DESTINATION lib${SELECTED_LIB_SUFFIX}
        COMPONENT binary
    )
    install(EXPORT ${META_PROJECT_NAME}StaticTargets
        DESTINATION "share/${META_PROJECT_NAME}/cmake"
        EXPORT_LINK_INTERFACE_LIBRARIES
        COMPONENT cmake-config
    )
endif()

#export(EXPORT ${META_PROJECT_NAME}Targets EXPORT_LINK_INTERFACE_LIBRARIES)

# add install target for stripped libs
if(NOT TARGET install-binary-strip)
    add_custom_target(install-binary-strip
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
