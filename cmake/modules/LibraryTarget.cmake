cmake_minimum_required(VERSION 3.3.0 FATAL_ERROR)

if(NOT BASIC_PROJECT_CONFIG_DONE)
    message(FATAL_ERROR "Before including the LibraryTarget module, the BasicConfig module must be included.")
endif()
if(TARGET_CONFIG_DONE)
    message(FATAL_ERROR "Can not include LibraryTarget module when targets are already configured.")
endif()

# check whether project type is set correctly
if(("${META_PROJECT_TYPE}" STREQUAL "plugin") OR ("${META_PROJECT_TYPE}" STREQUAL "qtplugin"))
    set(META_IS_PLUGIN YES)
endif()
if((NOT "${META_PROJECT_TYPE}" STREQUAL "library") AND (NOT "${META_PROJECT_TYPE}" STREQUAL "") AND NOT META_IS_PLUGIN)
    message(FATAL_ERROR "The LibraryTarget CMake module is intended to be used for building library projects only (and not for applications).")
endif()

# determine whether library is header-only
if(SRC_FILES OR WIDGETS_FILES OR QML_FILES OR RES_FILES)
    set(META_HEADER_ONLY_LIB NO)
else()
    message(STATUS "Project ${META_PROJECT_NAME} is header-only library.")
    set(META_HEADER_ONLY_LIB YES)
endif()

# includes for configure_package_config_file, write_basic_package_version_file and find_template_file
include(CMakePackageConfigHelpers)
include(TemplateFinder)

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
set(QT_PLUGIN_INSTALL_DESTINATION "${CMAKE_INSTALL_PREFIX}/lib${SELECTED_LIB_SUFFIX}/qt/plugins")
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
if(NOT META_SOVERSION AND NOT META_IS_PLUGIN)
    if(META_VERSION_EXACT_SONAME)
        set(META_SOVERSION "${META_VERSION_MAJOR}.${META_VERSION_MINOR}.${META_VERSION_PATCH}")
    else()
        set(META_SOVERSION "${META_VERSION_MAJOR}")
    endif()
endif()
message(STATUS "${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}: BUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}")

# define relevant files
set(ALL_FILES ${HEADER_FILES} ${SRC_FILES} ${GENERATED_DBUS_FILES} ${WIDGETS_FILES} ${QML_FILES} ${RES_FILES} ${WINDOWS_ICON_PATH})
if(NOT BUILTIN_TRANSLATIONS)
    list(APPEND ALL_FILES ${QM_FILES})
endif()

# determine include path used when building the project itself
if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include")
    # use special include directory if available
    set(TARGET_INCLUDE_DIRECTORY_BUILD_INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
else()
    # use the project folder itself
    set(TARGET_INCLUDE_DIRECTORY_BUILD_INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/..")
endif()

# add target for building the library
if(BUILD_SHARED_LIBS)
    if(STATIC_LIBRARY_LINKAGE)
        set(ACTUAL_ADDITIONAL_LINK_FLAGS ${META_ADDITIONAL_STATIC_LINK_FLAGS})
    else()
        set(ACTUAL_ADDITIONAL_LINK_FLAGS ${META_ADDITIONAL_SHARED_LINK_FLAGS})
    endif()
    if(META_IS_PLUGIN)
        set(META_SHARED_OBJECT_TYPE MODULE)
    else()
        set(META_SHARED_OBJECT_TYPE SHARED)
    endif()
    # add library to be created, set libs to link against, set version and C++ standard
    if(META_HEADER_ONLY_LIB)
        add_library(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX} INTERFACE)
        target_link_libraries(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}
            INTERFACE ${ACTUAL_ADDITIONAL_LINK_FLAGS} "${PUBLIC_LIBRARIES}" "${PRIVATE_LIBRARIES}"
        )
        target_include_directories(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}
            INTERFACE
                $<BUILD_INTERFACE:${TARGET_INCLUDE_DIRECTORY_BUILD_INTERFACE}>
                $<INSTALL_INTERFACE:${HEADER_INSTALL_DESTINATION}>
                ${PUBLIC_SHARED_INCLUDE_DIRS}
        )
        target_compile_definitions(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}
            INTERFACE "${META_PUBLIC_SHARED_LIB_COMPILE_DEFINITIONS}" "${META_PRIVATE_SHARED_LIB_COMPILE_DEFINITIONS}"
        )
        target_compile_options(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}
            INTERFACE "${META_PUBLIC_SHARED_LIB_COMPILE_OPTIONS}" "${META_PRIVATE_SHARED_LIB_COMPILE_OPTIONS}"
        )
    else()
        add_library(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX} ${META_SHARED_OBJECT_TYPE} ${ALL_FILES})
        target_link_libraries(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}
            PUBLIC ${ACTUAL_ADDITIONAL_LINK_FLAGS} "${PUBLIC_LIBRARIES}"
            PRIVATE "${PRIVATE_LIBRARIES}"
        )
        target_include_directories(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}
            PUBLIC
                $<BUILD_INTERFACE:${TARGET_INCLUDE_DIRECTORY_BUILD_INTERFACE}>
                $<INSTALL_INTERFACE:${HEADER_INSTALL_DESTINATION}>
                ${PUBLIC_SHARED_INCLUDE_DIRS}
            PRIVATE
                "${PRIVATE_SHARED_INCLUDE_DIRS}"
        )
        target_compile_definitions(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}
            PUBLIC "${META_PUBLIC_SHARED_LIB_COMPILE_DEFINITIONS}"
            PRIVATE "${META_PRIVATE_SHARED_LIB_COMPILE_DEFINITIONS}"
        )
        target_compile_options(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}
            PUBLIC "${META_PUBLIC_SHARED_LIB_COMPILE_OPTIONS}"
            PRIVATE "${META_PRIVATE_SHARED_LIB_COMPILE_OPTIONS}"
        )
        set_target_properties(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX} PROPERTIES
            VERSION "${META_VERSION_MAJOR}.${META_VERSION_MINOR}.${META_VERSION_PATCH}"
            SOVERSION "${META_SOVERSION}"
            CXX_STANDARD "${META_CXX_STANDARD}"
            LINK_SEARCH_START_STATIC ${STATIC_LINKAGE}
            LINK_SEARCH_END_STATIC ${STATIC_LINKAGE}
            AUTOGEN_TARGET_DEPENDS "${AUTOGEN_DEPS}"
        )
    endif()
endif()

# add target for building a static version of the library
if(BUILD_STATIC_LIBS)
    # add library to be created, set required libs, set version and C++ standard
    if(META_HEADER_ONLY_LIB)
        add_library(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_static INTERFACE)
        target_link_libraries(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_static
            INTERFACE ${ACTUAL_ADDITIONAL_LINK_FLAGS} "${PUBLIC_STATIC_LIBRARIES}" "${PRIVATE_STATIC_LIBRARIES}"
        )
        target_include_directories(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_static
            INTERFACE
                $<BUILD_INTERFACE:${TARGET_INCLUDE_DIRECTORY_BUILD_INTERFACE}>
                $<INSTALL_INTERFACE:${HEADER_INSTALL_DESTINATION}>
                ${PUBLIC_STATIC_INCLUDE_DIRS}
        )
        target_compile_definitions(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_static
            INTERFACE "${META_PUBLIC_STATIC_LIB_COMPILE_DEFINITIONS}" "${META_PRIVATE_STATIC_LIB_COMPILE_DEFINITIONS}"
        )
        target_compile_options(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_static
            INTERFACE "${META_PUBLIC_STATIC_LIB_COMPILE_OPTIONS}" "${META_PRIVATE_STATIC_LIB_COMPILE_OPTIONS}"
        )
    else()
        add_library(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_static STATIC ${ALL_FILES})
        target_link_libraries(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_static
            PUBLIC "${PUBLIC_STATIC_LIBRARIES}" "${PRIVATE_STATIC_LIBRARIES}"
        )
        target_include_directories(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_static
            PUBLIC
                $<BUILD_INTERFACE:${TARGET_INCLUDE_DIRECTORY_BUILD_INTERFACE}>
                $<INSTALL_INTERFACE:${HEADER_INSTALL_DESTINATION}>
                ${PUBLIC_STATIC_INCLUDE_DIRS}
            PRIVATE
                "${PRIVATE_STATIC_INCLUDE_DIRS}"
        )
        target_compile_definitions(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_static
            PUBLIC "${META_PUBLIC_STATIC_LIB_COMPILE_DEFINITIONS}"
            PRIVATE "${META_PRIVATE_STATIC_LIB_COMPILE_DEFINITIONS}"
        )
        target_compile_options(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_static
            PUBLIC "${META_PUBLIC_STATIC_LIB_COMPILE_OPTIONS}"
            PRIVATE "${META_PRIVATE_STATIC_LIB_COMPILE_OPTIONS}"
        )
        set_target_properties(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_static PROPERTIES
            VERSION "${META_VERSION_MAJOR}.${META_VERSION_MINOR}.${META_VERSION_PATCH}"
            SOVERSION "${META_SOVERSION}"
            OUTPUT_NAME "${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}"
            CXX_STANDARD "${META_CXX_STANDARD}"
            AUTOGEN_TARGET_DEPENDS "${AUTOGEN_DEPS}"
        )
    endif()
    foreach(DEPENDENCY ${PUBLIC_STATIC_LIBRARIES} ${PRIVATE_STATIC_LIBRARIES})
        if(NOT ${DEPENDENCY} IN_LIST META_PUBLIC_STATIC_LIB_DEPENDS)
            list(APPEND META_PRIVATE_STATIC_LIB_DEPENDS ${DEPENDENCY})
        endif()
    endforeach()
endif()

# Qt Creator does not show INTERFACE_SOURCES in project tree, so create a custom target as workaround
if(META_HEADER_ONLY_LIB)
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/headeronly.cpp" "// not belonging to a real target, only for header-only lib files showing up in Qt Creator")
    add_library(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_interface_sources_for_qtcreator
        EXCLUDE_FROM_ALL
        "${CMAKE_CURRENT_BINARY_DIR}/headeronly.cpp" ${HEADER_FILES}
    )
    target_include_directories(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_interface_sources_for_qtcreator
        INTERFACE
            $<BUILD_INTERFACE:${TARGET_INCLUDE_DIRECTORY_BUILD_INTERFACE}>
            $<INSTALL_INTERFACE:${HEADER_INSTALL_DESTINATION}>
            ${PUBLIC_SHARED_INCLUDE_DIRS}
    )
    target_compile_definitions(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_interface_sources_for_qtcreator
        INTERFACE "${META_PUBLIC_SHARED_LIB_COMPILE_DEFINITIONS}" "${META_PRIVATE_SHARED_LIB_COMPILE_DEFINITIONS}"
    )
    target_compile_options(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_interface_sources_for_qtcreator
        INTERFACE "${META_PUBLIC_SHARED_LIB_COMPILE_OPTIONS}" "${META_PRIVATE_SHARED_LIB_COMPILE_OPTIONS}"
    )
    set_target_properties(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_interface_sources_for_qtcreator PROPERTIES
        VERSION "${META_VERSION_MAJOR}.${META_VERSION_MINOR}.${META_VERSION_PATCH}"
        SOVERSION "${META_SOVERSION}"
        CXX_STANDARD "${META_CXX_STANDARD}"
        AUTOGEN_TARGET_DEPENDS "${AUTOGEN_DEPS}"
    )
endif()

# create the CMake package config file from template
find_template_file("Config.cmake" CPP_UTILITIES CONFIG_TEMPLATE_FILE)
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
list(APPEND CMAKE_CONFIG_FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${META_PROJECT_NAME}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${META_PROJECT_NAME}ConfigVersion.cmake"
)
if(BUILD_SHARED_LIBS)
    find_template_file("SharedConfig.cmake" CPP_UTILITIES SHARED_CONFIG_TEMPLATE_FILE)
    configure_file(
        "${SHARED_CONFIG_TEMPLATE_FILE}"
        "${CMAKE_CURRENT_BINARY_DIR}/${META_PROJECT_NAME}SharedConfig.cmake"
        @ONLY
    )
    list(APPEND CMAKE_CONFIG_FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${META_PROJECT_NAME}SharedConfig.cmake"
    )
endif()
if(BUILD_STATIC_LIBS)
    find_template_file("StaticConfig.cmake" CPP_UTILITIES STATIC_CONFIG_TEMPLATE_FILE)
    configure_file(
        "${STATIC_CONFIG_TEMPLATE_FILE}"
        "${CMAKE_CURRENT_BINARY_DIR}/${META_PROJECT_NAME}StaticConfig.cmake"
        @ONLY
    )
    list(APPEND CMAKE_CONFIG_FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${META_PROJECT_NAME}StaticConfig.cmake"
    )
endif()

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
        if(PC_PKG_${LIB_TYPE}_${DEPENDENCY_VARNAME})
            set(${OUTPUT_VAR_PKGS} "${${OUTPUT_VAR_PKGS}} ${PC_PKG_${LIB_TYPE}_${DEPENDENCY_VARNAME}}")
        else()
            set(${OUTPUT_VAR_LIBS} "${${OUTPUT_VAR_LIBS}} ${DEPENDENCY}")
        endif()
    endforeach()
endmacro()
macro(compile_defs_for_pc LIB_TYPE)
    foreach(COMPILE_DEFINITION ${META_PUBLIC_${LIB_TYPE}_LIB_COMPILE_DEFINITIONS})
        set(META_COMPILE_DEFINITIONS_FOR_PC "${META_COMPILE_DEFINITIONS_FOR_PC} -D${COMPILE_DEFINITION}")
    endforeach()
endmacro()
unset(PC_FILES)
if(BUILD_SHARED_LIBS)
    set(META_PROJECT_NAME_FOR_PC "${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}")
    depends_for_pc(SHARED META_PUBLIC_SHARED_LIB_DEPENDS META_PUBLIC_PC_PKGS META_PUBLIC_LIB_DEPENDS_FOR_PC)
    depends_for_pc(SHARED META_PRIVATE_SHARED_LIB_DEPENDS META_PRIVATE_PC_PKGS META_PRIVATE_LIB_DEPENDS_FOR_PC)
    compile_defs_for_pc(SHARED)
    if(NOT META_HEADER_ONLY_LIB)
        set(META_PUBLIC_LIB_DEPENDS_FOR_PC " -l${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}${META_PUBLIC_LIB_DEPENDS_FOR_PC}")
    endif()
    if(META_PUBLIC_LIB_DEPENDS_FOR_PC)
        set(META_PUBLIC_LIB_DEPENDS_FOR_PC " -L\${libdir}${META_PUBLIC_LIB_DEPENDS_FOR_PC}")
    endif()
    configure_file(
        "${PKGCONFIG_TEMPLATE_FILE}"
        "${CMAKE_CURRENT_BINARY_DIR}/${META_PROJECT_NAME_FOR_PC}.pc"
        @ONLY
    )
    list(APPEND PC_FILES "${CMAKE_CURRENT_BINARY_DIR}/${META_PROJECT_NAME_FOR_PC}.pc")
endif()
if(BUILD_STATIC_LIBS)
    set(META_PROJECT_NAME_FOR_PC "${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_static")
    depends_for_pc(STATIC META_PUBLIC_STATIC_LIB_DEPENDS META_PUBLIC_PC_PKGS META_PUBLIC_LIB_DEPENDS_FOR_PC)
    depends_for_pc(STATIC META_PRIVATE_STATIC_LIB_DEPENDS META_PRIVATE_PC_PKGS META_PRIVATE_LIB_DEPENDS_FOR_PC)
    compile_defs_for_pc(STATIC)
    if(NOT META_HEADER_ONLY_LIB)
        set(META_PUBLIC_LIB_DEPENDS_FOR_PC " -l${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}${META_PUBLIC_LIB_DEPENDS_FOR_PC}")
    endif()
    if(META_PUBLIC_LIB_DEPENDS_FOR_PC)
        set(META_PUBLIC_LIB_DEPENDS_FOR_PC " -L\${libdir}${META_PUBLIC_LIB_DEPENDS_FOR_PC}")
    endif()
    configure_file(
        "${PKGCONFIG_TEMPLATE_FILE}"
        "${CMAKE_CURRENT_BINARY_DIR}/${META_PROJECT_NAME_FOR_PC}.pc"
        @ONLY
    )
    list(APPEND PC_FILES "${CMAKE_CURRENT_BINARY_DIR}/${META_PROJECT_NAME_FOR_PC}.pc")
endif()

if(NOT META_NO_INSTALL_TARGETS AND ENABLE_INSTALL_TARGETS)
    # add install target for the CMake config files
    install(
        FILES ${CMAKE_CONFIG_FILES}
        DESTINATION "share/${META_PROJECT_NAME}/cmake"
        COMPONENT cmake-config
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

    # add install target for libs
    if(NOT TARGET install-binary)
        add_custom_target(install-binary
            COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=binary -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
        )
    endif()

    # add install target for stripped libs
    if(NOT TARGET install-binary-strip)
        add_custom_target(install-binary-strip
            COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_DO_STRIP=1 -DCMAKE_INSTALL_COMPONENT=binary -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
        )
    endif()

    # determine install dir for Qt plugins
    if("${META_PROJECT_TYPE}" STREQUAL "qtplugin")
        if(QT_PLUGIN_DIR)
            set(LIBRARY_DESTINATION ${QT_PLUGIN_DIR})
        else()
            set(LIBRARY_DESTINATION lib${SELECTED_LIB_SUFFIX}/qt/plugins)
        endif()
        if(META_PLUGIN_CATEGORY)
            set(LIBRARY_DESTINATION ${LIBRARY_DESTINATION}/${META_PLUGIN_CATEGORY})
        endif()
    else()
        set(LIBRARY_DESTINATION lib${SELECTED_LIB_SUFFIX})
    endif()

    # add install targets and export targets
    if(BUILD_SHARED_LIBS)
        install(
            TARGETS ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}
            EXPORT ${META_PROJECT_NAME}SharedTargets
            RUNTIME DESTINATION bin
            COMPONENT binary
            LIBRARY DESTINATION ${LIBRARY_DESTINATION}
            COMPONENT binary
            ARCHIVE DESTINATION ${LIBRARY_DESTINATION}
            COMPONENT binary
        )
        add_dependencies(install-binary ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX})
        add_dependencies(install-binary-strip ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX})
        # export shared lib
        install(EXPORT ${META_PROJECT_NAME}SharedTargets
            DESTINATION "share/${META_PROJECT_NAME}/cmake"
            EXPORT_LINK_INTERFACE_LIBRARIES
            COMPONENT cmake-config
        )
    endif()
    if(BUILD_STATIC_LIBS)
        install(
            TARGETS ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_static
            EXPORT ${META_PROJECT_NAME}StaticTargets
            RUNTIME DESTINATION bin
            COMPONENT binary
            LIBRARY DESTINATION ${LIBRARY_DESTINATION}
            COMPONENT binary
            ARCHIVE DESTINATION ${LIBRARY_DESTINATION}
            COMPONENT binary
        )
        add_dependencies(install-binary ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_static)
        add_dependencies(install-binary-strip ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_static)
        # export static target
        install(EXPORT ${META_PROJECT_NAME}StaticTargets
            DESTINATION "share/${META_PROJECT_NAME}/cmake"
            EXPORT_LINK_INTERFACE_LIBRARIES
            COMPONENT cmake-config
        )
    endif()

    # add install target for header files
    if(NOT META_IS_PLUGIN)
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
        add_custom_target(install-cmake-stuff)
        add_dependencies(install-cmake-stuff install-cmake-config install-cmake-modules install-cmake-templates)
    endif()

    # add mingw-w64 specific install targets
    if(NOT TARGET install-mingw-w64)
        add_custom_target(install-mingw-w64)
    endif()
    add_dependencies(install-mingw-w64 install-binary install-header install-cmake-stuff install-pkg-config)
    if(NOT TARGET install-mingw-w64-strip)
        add_custom_target(install-mingw-w64-strip)
    endif()
    add_dependencies(install-mingw-w64-strip install-binary-strip install-header install-cmake-stuff install-pkg-config)
    if(LOCALIZATION_TARGET)
        add_dependencies(install-mingw-w64 ${LOCALIZATION_TARGET})
        add_dependencies(install-mingw-w64-strip ${LOCALIZATION_TARGET})
    endif()
    if(BUILD_SHARED_LIBS AND NOT META_HEADER_ONLY_LIB)
        add_custom_target(install-${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}-mingw-w64-importlib-strip
            COMMAND "${CMAKE_FIND_ROOT_PATH}/bin/strip" -g "\$\{DESTDIR\}\$\{DESTDIR:+/\}${CMAKE_INSTALL_PREFIX}/lib/lib${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}.dll.a"
        )
        add_dependencies(install-${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}-mingw-w64-importlib-strip install-binary-strip)
        add_dependencies(install-mingw-w64-strip install-${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}-mingw-w64-importlib-strip)
    endif()
    if(BUILD_STATIC_LIBS AND NOT META_HEADER_ONLY_LIB)
        add_custom_target(install-${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}-mingw-w64-staticlib-strip
            COMMAND "${CMAKE_FIND_ROOT_PATH}/bin/strip" -g "\$\{DESTDIR\}\$\{DESTDIR:+/\}${CMAKE_INSTALL_PREFIX}/lib/lib${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}.a"
        )
        add_dependencies(install-${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}-mingw-w64-staticlib-strip install-binary-strip)
        add_dependencies(install-mingw-w64-strip install-${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}-mingw-w64-staticlib-strip)
    endif()
endif()

set(TARGET_CONFIG_DONE YES)
