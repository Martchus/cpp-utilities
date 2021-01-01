cmake_minimum_required(VERSION 3.3.0 FATAL_ERROR)

if (NOT BASIC_PROJECT_CONFIG_DONE)
    message(FATAL_ERROR "Before including the LibraryTarget module, the BasicConfig module must be included.")
endif ()
if (TARGET_CONFIG_DONE)
    message(FATAL_ERROR "Can not include LibraryTarget module when targets are already configured.")
endif ()

# check whether project type is set correctly
if (("${META_PROJECT_TYPE}" STREQUAL "plugin") OR ("${META_PROJECT_TYPE}" STREQUAL "qtplugin"))
    set(META_IS_PLUGIN YES)
endif ()
if ((NOT "${META_PROJECT_TYPE}" STREQUAL "library")
    AND (NOT "${META_PROJECT_TYPE}" STREQUAL "")
    AND NOT META_IS_PLUGIN)
    message(
        FATAL_ERROR
            "The LibraryTarget CMake module is intended to be used for building library projects only (and not for applications)."
    )
endif ()

# add option for enabling versioned mingw-w64 libraries (disabled by default to preserve compatibility, will be the only
# naming scheme in v6)
option(VERSIONED_MINGW_LIBRARIES
       "enables versioned libraries like 'libc++utilities-5.dll' instead of 'c++utilities.dll' for mingw-w64 targets" OFF)

# include packages for configure_package_config_file, write_basic_package_version_file and find_template_file
include(CMakePackageConfigHelpers)
include(TemplateFinder)

# set install destination for the CMake modules, config files and header files
set(INCLUDE_SUBDIR "${CMAKE_INSTALL_INCLUDEDIR}")
set(HEADER_INSTALL_DESTINATION "${CMAKE_INSTALL_FULL_INCLUDEDIR}")
if (META_CONFIG_SUFFIX)
    set(INCLUDE_SUBDIR "${INCLUDE_SUBDIR}/${META_PROJECT_NAME}${META_CONFIG_SUFFIX}")
    set(HEADER_INSTALL_DESTINATION "${HEADER_INSTALL_DESTINATION}/${META_PROJECT_NAME}${META_CONFIG_SUFFIX}")
endif ()
set(CMAKE_MODULE_INSTALL_DESTINATION "${META_DATA_DIR_ABSOLUTE}/cmake/modules")
set(CMAKE_CONFIG_INSTALL_DESTINATION "${META_DATA_DIR_ABSOLUTE}/cmake")

# remove library prefix when building with mingw-w64 (just for consistency with qmake)
if (MINGW AND NOT VERSIONED_MINGW_LIBRARIES)
    set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif ()

# set compile definitions for static build
if (NOT BUILD_SHARED_LIBS)
    list(APPEND META_PUBLIC_COMPILE_DEFINITIONS ${META_PROJECT_VARNAME_UPPER}_STATIC)
endif ()

# add global library-specific header
find_template_file("global.h" CPP_UTILITIES GLOBAL_H_TEMPLATE_FILE)
if ("${META_PROJECT_NAME}" STREQUAL "c++utilities")
    set(GENERAL_GLOBAL_H_INCLUDE_PATH "\"./application/global.h\"")
else ()
    set(GENERAL_GLOBAL_H_INCLUDE_PATH "<c++utilities/application/global.h>")
endif ()
configure_file(
    "${GLOBAL_H_TEMPLATE_FILE}" "${CMAKE_CURRENT_SOURCE_DIR}/global.h" # simply add this to source to ease inclusion
    NEWLINE_STYLE UNIX # since this goes to sources ensure consistency
)
list(APPEND HEADER_FILES global.h)

# add header to check library version
set(VERSION_HEADER_FILE "${CMAKE_CURRENT_BINARY_DIR}/resources/version.h")
find_template_file("version.h" CPP_UTILITIES VERSION_H_TEMPLATE_FILE)
configure_file("${VERSION_H_TEMPLATE_FILE}" "${VERSION_HEADER_FILE}" NEWLINE_STYLE UNIX)
list(APPEND SOURCE_FILES "${VERSION_HEADER_FILE}")

# determine SOVERSION
if (NOT META_SOVERSION AND NOT META_IS_PLUGIN)
    if (META_VERSION_EXACT_SONAME)
        set(META_SOVERSION "${META_VERSION_MAJOR}.${META_VERSION_MINOR}.${META_VERSION_PATCH}")
    else ()
        set(META_SOVERSION "${META_VERSION_MAJOR}")
    endif ()
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

# determine include path used when building the project itself
if (TARGET_INCLUDE_DIRECTORY_BUILD_INTERFACE)
    # use existing TARGET_INCLUDE_DIRECTORY_BUILD_INTERFACE if already defined
elseif (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include")
    # use special include directory if available
    set(TARGET_INCLUDE_DIRECTORY_BUILD_INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
else ()
    # use the project folder itself
    set(TARGET_INCLUDE_DIRECTORY_BUILD_INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/..")
endif ()

# add target for building the library
if (BUILD_SHARED_LIBS)
    if (META_IS_PLUGIN)
        set(META_LIBRARY_TYPE MODULE)
    else ()
        set(META_LIBRARY_TYPE SHARED)
    endif ()
else ()
    set(META_LIBRARY_TYPE STATIC)
endif ()

# add custom libraries
append_user_defined_additional_libraries()

# add library to be created, set libs to link against, set version and C++ standard
if (META_HEADER_ONLY_LIB)
    add_library(${META_TARGET_NAME} INTERFACE)
    target_link_libraries(${META_TARGET_NAME} INTERFACE ${META_ADDITIONAL_LINK_FLAGS} "${PUBLIC_LIBRARIES}"
                                                        "${PRIVATE_LIBRARIES}")
    target_include_directories(
        ${META_TARGET_NAME} INTERFACE $<BUILD_INTERFACE:${TARGET_INCLUDE_DIRECTORY_BUILD_INTERFACE}>
                                      $<INSTALL_INTERFACE:${HEADER_INSTALL_DESTINATION}> ${PUBLIC_INCLUDE_DIRS})
    target_compile_definitions(${META_TARGET_NAME} INTERFACE "${META_PUBLIC_COMPILE_DEFINITIONS}"
                                                             "${META_PRIVATE_COMPILE_DEFINITIONS}")
    target_compile_options(${META_TARGET_NAME} INTERFACE "${META_PUBLIC_COMPILE_OPTIONS}" "${META_PRIVATE_COMPILE_OPTIONS}")
else ()
    add_library(${META_TARGET_NAME} ${META_LIBRARY_TYPE} ${ALL_FILES})
    target_link_libraries(
        ${META_TARGET_NAME}
        PUBLIC ${META_ADDITIONAL_LINK_FLAGS} "${PUBLIC_LIBRARIES}"
        PRIVATE "${PRIVATE_LIBRARIES}")
    target_include_directories(
        ${META_TARGET_NAME}
        PUBLIC $<BUILD_INTERFACE:${TARGET_INCLUDE_DIRECTORY_BUILD_INTERFACE}>
               $<INSTALL_INTERFACE:${HEADER_INSTALL_DESTINATION}> ${PUBLIC_INCLUDE_DIRS}
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
        PROPERTIES VERSION "${META_VERSION_MAJOR}.${META_VERSION_MINOR}.${META_VERSION_PATCH}"
                   SOVERSION "${META_SOVERSION}"
                   CXX_STANDARD "${META_CXX_STANDARD}"
                   C_VISIBILITY_PRESET hidden
                   CXX_VISIBILITY_PRESET hidden
                   LINK_SEARCH_START_STATIC ${STATIC_LINKAGE}
                   LINK_SEARCH_END_STATIC ${STATIC_LINKAGE}
                   AUTOGEN_TARGET_DEPENDS "${AUTOGEN_DEPS}"
                   QT_DEFAULT_PLUGINS "${META_QT_DEFAULT_PLUGINS}")
    if (META_PLUGIN_CATEGORY)
        set_target_properties(${META_TARGET_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${META_PLUGIN_CATEGORY}")
    endif ()

    # incorporate the SOVERSION into the library name for mingw-w64 targets
    if (BUILD_SHARED_LIBS
        AND NOT META_IS_PLUGIN
        AND MINGW
        AND VERSIONED_MINGW_LIBRARIES)
        set_target_properties(${META_TARGET_NAME} PROPERTIES SUFFIX "-${META_SOVERSION}.dll")
    endif ()

    # avoid duplicating the "lib" prefix if the target name already starts with "lib"
    if (META_TARGET_NAME MATCHES "lib.*")
        set_target_properties(${META_TARGET_NAME} PROPERTIES PREFIX "")
    endif ()

    # add target for pulling only headers because some libraries contain header-only parts which are useful on their own
    if (NOT META_PLUGIN_CATEGORY)
        add_library(${META_TARGET_NAME}-headers INTERFACE)
        target_include_directories(
            ${META_TARGET_NAME}-headers INTERFACE $<BUILD_INTERFACE:${TARGET_INCLUDE_DIRECTORY_BUILD_INTERFACE}>
                                                  $<INSTALL_INTERFACE:${HEADER_INSTALL_DESTINATION}> ${PUBLIC_INCLUDE_DIRS})
        target_compile_definitions(${META_TARGET_NAME}-headers INTERFACE "${META_PUBLIC_COMPILE_DEFINITIONS}"
                                                                         "${META_PRIVATE_COMPILE_DEFINITIONS}")
        target_compile_options(${META_TARGET_NAME}-headers INTERFACE "${META_PUBLIC_COMPILE_OPTIONS}"
                                                                     "${META_PRIVATE_COMPILE_OPTIONS}")
    endif ()
endif ()

# populate META_PUBLIC_LIB_DEPENDS
foreach (DEPENDENCY ${PUBLIC_LIBRARIES})
    if (NOT "${DEPENDENCY}" IN_LIST META_PUBLIC_LIB_DEPENDS)
        list(APPEND META_PUBLIC_LIB_DEPENDS ${DEPENDENCY})
    endif ()
endforeach ()
# populate META_PRIVATE_LIB_DEPENDS (only required when building static libraries)
if (NOT BUILD_SHARED_LIBS)
    foreach (DEPENDENCY ${PRIVATE_LIBRARIES})
        if (NOT "${DEPENDENCY}" IN_LIST META_PUBLIC_LIB_DEPENDS AND NOT "${DEPENDENCY}" IN_LIST META_PRIVATE_LIB_DEPENDS)
            list(APPEND META_PRIVATE_LIB_DEPENDS ${DEPENDENCY})
        endif ()
    endforeach ()
endif ()

# Qt Creator does not show INTERFACE_SOURCES in project tree, so create a custom target as workaround
if (META_HEADER_ONLY_LIB)
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/headeronly.cpp"
         "// not belonging to a real target, only for header-only lib files showing up in Qt Creator")
    add_library(${META_TARGET_NAME}_interface_sources_for_qtcreator EXCLUDE_FROM_ALL
                "${CMAKE_CURRENT_BINARY_DIR}/headeronly.cpp" ${HEADER_FILES})
    target_include_directories(
        ${META_TARGET_NAME}_interface_sources_for_qtcreator
        INTERFACE $<BUILD_INTERFACE:${TARGET_INCLUDE_DIRECTORY_BUILD_INTERFACE}>
                  $<INSTALL_INTERFACE:${HEADER_INSTALL_DESTINATION}> ${PUBLIC_INCLUDE_DIRS})
    target_compile_definitions(${META_TARGET_NAME}_interface_sources_for_qtcreator
                               INTERFACE "${META_PUBLIC_COMPILE_DEFINITIONS}" "${META_PRIVATE_COMPILE_DEFINITIONS}")
    target_compile_options(${META_TARGET_NAME}_interface_sources_for_qtcreator INTERFACE "${META_PUBLIC_COMPILE_OPTIONS}"
                                                                                         "${META_PRIVATE_COMPILE_OPTIONS}")
    set_target_properties(
        ${META_TARGET_NAME}_interface_sources_for_qtcreator
        PROPERTIES VERSION "${META_VERSION_MAJOR}.${META_VERSION_MINOR}.${META_VERSION_PATCH}"
                   SOVERSION "${META_SOVERSION}"
                   CXX_STANDARD "${META_CXX_STANDARD}"
                   AUTOGEN_TARGET_DEPENDS "${AUTOGEN_DEPS}")
endif ()

# generate CMake code to configure additional arguments for required CMake-packages
set(ADDITIONAL_ARGUMENTS_FOR_REQUIRED_CMAKE_PACKAGES)
foreach (INTERFACE_REQUIRED_PACKAGE ${INTERFACE_REQUIRED_PACKAGES})
    if (PACKAGE_ARGS_${INTERFACE_REQUIRED_PACKAGE})
        set(ADDITIONAL_ARGUMENTS_FOR_REQUIRED_CMAKE_PACKAGES
            "${ADDITIONAL_ARGUMENTS_FOR_REQUIRED_CMAKE_PACKAGES}set(${META_PROJECT_VARNAME_UPPER}_FIND_PACKAGE_ARGS_${INTERFACE_REQUIRED_PACKAGE} \"${PACKAGE_ARGS_${INTERFACE_REQUIRED_PACKAGE}}\")\n"
        )
    endif ()
endforeach ()

# generate CMake code to configure CMake-target to pkg-config module mapping
set(TARGET_TO_PKG_CONFIG_MODULE_NAME_MAPPING
    "set(PKG_CONFIG_${META_TARGET_NAME} \"${META_PROJECT_NAME}${META_CONFIG_SUFFIX}\")")
foreach (INTERFACE_REQUIRED_PKG_CONFIG_MODULE ${INTERFACE_REQUIRED_PKG_CONFIG_MODULES})
    string(REPLACE "::" "_" INTERFACE_REQUIRED_PKG_CONFIG_MODULE_VARNAME "${INTERFACE_REQUIRED_PKG_CONFIG_MODULE}")
    set(TARGET_TO_PKG_CONFIG_MODULE_NAME_MAPPING
        "${TARGET_TO_PKG_CONFIG_MODULE_NAME_MAPPING}\nset(PKG_CONFIG_${INTERFACE_REQUIRED_PKG_CONFIG_MODULE_VARNAME} \"${PKG_CONFIG_${INTERFACE_REQUIRED_PKG_CONFIG_MODULE_VARNAME}}\")"
    )
endforeach ()

# create the CMake package config file from template
if (INTERFACE_REQUIRED_PACKAGES)
    list(REMOVE_ITEM INTERFACE_REQUIRED_PACKAGES "")
    list(REMOVE_DUPLICATES INTERFACE_REQUIRED_PACKAGES)
endif ()
if (INTERFACE_REQUIRED_PKG_CONFIG_MODULES)
    list(REMOVE_ITEM INTERFACE_REQUIRED_PKG_CONFIG_MODULES "")
    list(REMOVE_DUPLICATES INTERFACE_REQUIRED_PKG_CONFIG_MODULES)
endif ()
set(CONFIG_TARGETS "${CMAKE_CURRENT_BINARY_DIR}/${META_PROJECT_NAME}${META_CONFIG_SUFFIX}Config.cmake")
if (META_CONFIG_SUFFIX)
    list(APPEND CONFIG_TARGETS "${CMAKE_CURRENT_BINARY_DIR}/${META_PROJECT_NAME}Config.cmake")
endif ()
find_template_file("Config.cmake" CPP_UTILITIES CONFIG_TEMPLATE_FILE)
foreach (CONFIG_TARGET ${CONFIG_TARGETS})
    configure_package_config_file(
        "${CONFIG_TEMPLATE_FILE}" "${CONFIG_TARGET}"
        INSTALL_DESTINATION "${CMAKE_CONFIG_INSTALL_DESTINATION}"
        PATH_VARS CMAKE_MODULE_INSTALL_DESTINATION CMAKE_CONFIG_INSTALL_DESTINATION HEADER_INSTALL_DESTINATION
                  BIN_INSTALL_DESTINATION LIB_INSTALL_DESTINATION)
endforeach ()
list(APPEND CMAKE_CONFIG_FILES "${CMAKE_CURRENT_BINARY_DIR}/${META_PROJECT_NAME}${META_CONFIG_SUFFIX}Config.cmake"
     "${CMAKE_CURRENT_BINARY_DIR}/${META_PROJECT_NAME}${META_CONFIG_SUFFIX}ConfigVersion.cmake")

# write the CMake version config file
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${META_PROJECT_NAME}${META_CONFIG_SUFFIX}ConfigVersion.cmake
    VERSION "${META_VERSION_MAJOR}.${META_VERSION_MINOR}.${META_VERSION_PATCH}"
    COMPATIBILITY SameMajorVersion)

# compute dependencies for pkg-config file
macro (compute_dependencies_for_package_config DEPENDS OUTPUT_VAR_PKGS OUTPUT_VAR_LIBS UNSET)
    if (UNSET)
        unset(${OUTPUT_VAR_PKGS})
        unset(${OUTPUT_VAR_LIBS})
    endif ()
    foreach (DEPENDENCY ${${DEPENDS}})
        if ("${DEPENDENCY}" STREQUAL "general")
            continue()
        endif ()
        # handle use of generator expressions (use BUILD_INTERFACE; ignore INSTALL_INTERFACE)
        if ("${DEPENDENCY}" MATCHES "\\\$\\<BUILD_INTERFACE:(.*)\\>")
            set(DEPENDENCY "${CMAKE_MATCH_1}")
        elseif ("${DEPENDENCY}" MATCHES "\\\$\\<INSTALL_INTERFACE:(.*)\\>")
            continue()
        endif ()
        # find the name of the pkg-config package for the depencency
        string(REPLACE "::" "_" DEPENDENCY_VARNAME "${DEPENDENCY}")
        if (PKG_CONFIG_${DEPENDENCY_VARNAME})
            # add pkg-config modules for the dependency
            foreach (PKG_CONFIG_MODULE ${PKG_CONFIG_${DEPENDENCY_VARNAME}})
                set(${OUTPUT_VAR_PKGS} "${${OUTPUT_VAR_PKGS}} ${PKG_CONFIG_MODULE}")
            endforeach ()
        elseif (TARGET "${DEPENDENCY}")
            # find the raw-library flags for the dependency add interface link libraries of the target
            get_target_property("${DEPENDENCY_VARNAME}_INTERFACE_LINK_LIBRARIES" "${DEPENDENCY}" "INTERFACE_LINK_LIBRARIES")
            set(${DEPENDENCY_VARNAME}_INTERFACE_LINK_LIBRARIES_EXISTING FALSE)
            set(${DEPENDENCY_VARNAME}_INTERFACE_LINK_LIBRARIES_TARGETS)
            foreach (LIBRARY ${${DEPENDENCY_VARNAME}_INTERFACE_LINK_LIBRARIES})
                if (TARGET ${LIBRARY})
                    list(APPEND ${DEPENDENCY_VARNAME}_INTERFACE_LINK_LIBRARIES_TARGETS ${LIBRARY})
                    set(${DEPENDENCY_VARNAME}_INTERFACE_LINK_LIBRARIES_EXISTING TRUE)
                elseif (EXISTS ${LIBRARY})
                    set(${OUTPUT_VAR_LIBS} "${${OUTPUT_VAR_LIBS}} ${LIBRARY}")
                    set(${DEPENDENCY_VARNAME}_INTERFACE_LINK_LIBRARIES_EXISTING TRUE)
                endif ()
            endforeach ()
            compute_dependencies_for_package_config("${DEPENDENCY_VARNAME}_INTERFACE_LINK_LIBRARIES_TARGETS"
                                                    "${OUTPUT_VAR_PKGS}" "${OUTPUT_VAR_LIBS}" NO)
            if (${DEPENDENCY_VARNAME}_INTERFACE_LINK_LIBRARIES_EXISTING)
                continue()
            endif ()
            # add library location of the target
            set(HAS_LIBRARY_LOCATION NO)
            if (DEPENDENCY IN_LIST BUNDLED_TARGETS)
                # bundled targets are installed at the same location and using the same flags as the actual library so adding
                # the library name itself is sufficient
                set(${OUTPUT_VAR_LIBS} "${${OUTPUT_VAR_LIBS}} -l${DEPENDENCY}")
                set(HAS_LIBRARY_LOCATION YES)
            endif ()
            if (NOT HAS_LIBRARY_LOCATION AND META_CURRENT_CONFIGURATION)
                get_target_property("${DEPENDENCY_VARNAME}_IMPORTED_LOCATION_${META_CURRENT_CONFIGURATION}" "${DEPENDENCY}"
                                    "IMPORTED_LOCATION_${META_CURRENT_CONFIGURATION}")
                # check whether path points to the build directory; libraries must not be referenced using their build
                # location
                string(FIND "${${DEPENDENCY_VARNAME}_IMPORTED_LOCATION_${META_CURRENT_CONFIGURATION}}"
                            "${CMAKE_CURRENT_BINARY_DIR}" BINARY_DIR_INDEX)
                if (NOT BINARY_DIR_INDEX EQUAL 0
                    AND EXISTS "${${DEPENDENCY_VARNAME}_IMPORTED_LOCATION_${META_CURRENT_CONFIGURATION}}")
                    set(${OUTPUT_VAR_LIBS}
                        "${${OUTPUT_VAR_LIBS}} ${${DEPENDENCY_VARNAME}_IMPORTED_LOCATION_${META_CURRENT_CONFIGURATION}}")
                    set(HAS_LIBRARY_LOCATION YES)
                endif ()
            endif ()
            if (NOT HAS_LIBRARY_LOCATION)
                get_target_property("${DEPENDENCY_VARNAME}_IMPORTED_LOCATION" "${DEPENDENCY}" IMPORTED_LOCATION)
                # check whether path points to the build directory; libraries must not be referenced using their build
                # location
                string(FIND "${${DEPENDENCY_VARNAME}_IMPORTED_LOCATION}" "${CMAKE_CURRENT_BINARY_DIR}" BINARY_DIR_INDEX)
                if (NOT BINARY_DIR_INDEX EQUAL 0 AND EXISTS "${${DEPENDENCY_VARNAME}_IMPORTED_LOCATION}")
                    set(${OUTPUT_VAR_LIBS} "${${OUTPUT_VAR_LIBS}} ${${DEPENDENCY_VARNAME}_IMPORTED_LOCATION}")
                    set(HAS_LIBRARY_LOCATION YES)
                endif ()
            endif ()
            # assume the target is a 3rd party library built within the current project as a bundled dependency -> the target
            # is supposed to be installed in either a standard search directory or the same directory as this library so a
            # simple -l flag should be sufficient
            if (NOT HAS_LIBRARY_LOCATION)
                set(${OUTPUT_VAR_LIBS} "${${OUTPUT_VAR_LIBS}} -l${DEPENDENCY}")
            endif ()
            # add libraries required by the imported target
            get_target_property("${DEPENDENCY_VARNAME}_IMPORTED_LINK_INTERFACE_LIBRARIES" "${DEPENDENCY}"
                                "IMPORTED_LINK_INTERFACE_LIBRARIES")
            compute_dependencies_for_package_config("${DEPENDENCY_VARNAME}_IMPORTED_LINK_INTERFACE_LIBRARIES"
                                                    "${OUTPUT_VAR_PKGS}" "${OUTPUT_VAR_LIBS}" NO)
        else ()
            # add raw dependency
            set(${OUTPUT_VAR_LIBS} "${${OUTPUT_VAR_LIBS}} ${DEPENDENCY}")
        endif ()
    endforeach ()
endmacro ()
compute_dependencies_for_package_config(META_PUBLIC_LIB_DEPENDS META_PUBLIC_PC_PKGS META_PUBLIC_LIB_DEPENDS_FOR_PC YES)
compute_dependencies_for_package_config(META_PRIVATE_LIB_DEPENDS META_PRIVATE_PC_PKGS META_PRIVATE_LIB_DEPENDS_FOR_PC YES)
if (NOT META_HEADER_ONLY_LIB)
    set(META_PUBLIC_LIB_DEPENDS_FOR_PC " -l${META_TARGET_NAME}${META_PUBLIC_LIB_DEPENDS_FOR_PC}")
endif ()
if (META_PUBLIC_LIB_DEPENDS_FOR_PC)
    set(META_PUBLIC_LIB_DEPENDS_FOR_PC " -L\${libdir}${META_PUBLIC_LIB_DEPENDS_FOR_PC}")
endif ()

# compute other values for pkg-config
set(META_PROJECT_NAME_FOR_PC "${META_PROJECT_NAME}${META_CONFIG_SUFFIX}")
foreach (COMPILE_DEFINITION ${META_PUBLIC_COMPILE_DEFINITIONS})
    set(META_COMPILE_DEFINITIONS_FOR_PC "${META_COMPILE_DEFINITIONS_FOR_PC} -D${COMPILE_DEFINITION}")
endforeach ()

# create pkg-config file from template using previously computed values
find_template_file("template.pc" CPP_UTILITIES PKGCONFIG_TEMPLATE_FILE)
configure_file("${PKGCONFIG_TEMPLATE_FILE}" "${CMAKE_CURRENT_BINARY_DIR}/${META_PROJECT_NAME_FOR_PC}.pc" @ONLY)
set(PC_FILES "${CMAKE_CURRENT_BINARY_DIR}/${META_PROJECT_NAME_FOR_PC}.pc")

# add install targets
if (NOT META_NO_INSTALL_TARGETS AND ENABLE_INSTALL_TARGETS)
    # add install target for the CMake config files
    install(
        FILES ${CMAKE_CONFIG_FILES}
        DESTINATION "${META_DATA_DIR}/cmake"
        COMPONENT cmake-config)
    if (NOT TARGET install-cmake-config)
        add_custom_target(install-cmake-config COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=cmake-config -P
                                                       "${CMAKE_BINARY_DIR}/cmake_install.cmake")
    endif ()

    # add install target for pkg-config file
    if (PC_FILES)
        install(
            FILES ${PC_FILES}
            DESTINATION "${CMAKE_INSTALL_LIBDIR}${SELECTED_LIB_SUFFIX}/pkgconfig"
            COMPONENT pkg-config)
    endif ()
    if (NOT TARGET install-pkg-config)
        add_custom_target(install-pkg-config COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=pkg-config -P
                                                     "${CMAKE_BINARY_DIR}/cmake_install.cmake")
    endif ()

    # add install target for libs
    if (NOT TARGET install-binary)
        add_custom_target(install-binary COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=binary -P
                                                 "${CMAKE_BINARY_DIR}/cmake_install.cmake")
    endif ()

    # add install target for stripped libs
    if (NOT TARGET install-binary-strip)
        add_custom_target(
            install-binary-strip COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_DO_STRIP=1 -DCMAKE_INSTALL_COMPONENT=binary -P
                                         "${CMAKE_BINARY_DIR}/cmake_install.cmake")
    endif ()

    # determine install dir for Qt plugins
    if ("${META_PROJECT_TYPE}" STREQUAL "qtplugin")
        if (QT_PLUGIN_DIR)
            set(LIBRARY_DESTINATION "${QT_PLUGIN_DIR}")
        else ()
            if (COMMAND query_qmake_variable_path)
                query_qmake_variable_path(QT_INSTALL_PLUGINS)
            elseif (COMMAND query_qmake_variable)
                query_qmake_variable(QT_INSTALL_PLUGINS)
            endif ()
            if (QT_INSTALL_PLUGINS)
                string(FIND "${QT_INSTALL_PLUGINS}" "${CMAKE_INSTALL_PREFIX}"
                            CMAKE_INSTALL_PREFIX_INDEX_IN_QT_INSTALL_PLUGINS)
                if ("${CMAKE_INSTALL_PREFIX_INDEX_IN_QT_INSTALL_PLUGINS}" EQUAL 0)
                    set(LIBRARY_DESTINATION ${QT_INSTALL_PLUGINS})
                else ()
                    message(
                        WARNING
                            "According to qmake the Qt plugin directory is \"${QT_INSTALL_PLUGINS}\". However, that path is not within the install prefix \"${CMAKE_INSTALL_PREFIX}\" and therefore ignored."
                    )
                endif ()
            endif ()
            if (NOT LIBRARY_DESTINATION)
                set(LIBRARY_DESTINATION ${CMAKE_INSTALL_LIBDIR}${SELECTED_LIB_SUFFIX}/qt/plugins)
                message(
                    WARNING
                        "Unable to detect appropriate install directory for Qt plugins (assuming \"${LIBRARY_DESTINATION}\"). Set QT_PLUGIN_DIR to specify the directory to install Qt plugins to manually."
                )
            endif ()
        endif ()
        if (META_PLUGIN_CATEGORY)
            set(LIBRARY_DESTINATION ${LIBRARY_DESTINATION}/${META_PLUGIN_CATEGORY})
        endif ()
    else ()
        set(LIBRARY_DESTINATION ${CMAKE_INSTALL_LIBDIR}${SELECTED_LIB_SUFFIX})
    endif ()

    # add install targets and export targets
    set(TARGETS_TO_EXPORT "${META_TARGET_NAME}")
    foreach (BUNDLED_TARGET ${BUNDLED_TARGETS})
        if (NOT ${BUNDLED_TARGET} IN_LIST LIBRARIES OR (NOT BUILD_SHARED_LIBS AND ${BUNDLED_TARGET} IN_LIST PRIVATE_LIBRARIES
                                                       ))
            list(APPEND TARGETS_TO_EXPORT ${BUNDLED_TARGET})
        endif ()
    endforeach ()
    if (NOT META_HEADER_ONLY_LIB AND NOT META_PLUGIN_CATEGORY)
        list(APPEND TARGETS_TO_EXPORT "${META_TARGET_NAME}-headers")
    endif ()
    install(
        TARGETS ${TARGETS_TO_EXPORT}
        EXPORT "${META_PROJECT_NAME}${META_CONFIG_SUFFIX}Targets"
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT binary
        LIBRARY DESTINATION ${LIBRARY_DESTINATION} COMPONENT binary
        ARCHIVE DESTINATION ${LIBRARY_DESTINATION} COMPONENT binary)
    add_dependencies(install-binary "${META_TARGET_NAME}")
    add_dependencies(install-binary-strip "${META_TARGET_NAME}")
    install(
        EXPORT ${META_PROJECT_NAME}${META_CONFIG_SUFFIX}Targets
        DESTINATION "${META_DATA_DIR}/cmake"
        EXPORT_LINK_INTERFACE_LIBRARIES
        COMPONENT cmake-config)

    # add install target for header files
    if (NOT META_IS_PLUGIN)
        foreach (HEADER_FILE ${HEADER_FILES} ${ADDITIONAL_HEADER_FILES})
            get_filename_component(HEADER_DIR "${HEADER_FILE}" DIRECTORY)
            install(
                FILES "${HEADER_FILE}"
                DESTINATION "${INCLUDE_SUBDIR}/${META_PROJECT_NAME}/${HEADER_DIR}"
                COMPONENT header)
        endforeach ()
        install(
            FILES "${VERSION_HEADER_FILE}"
            DESTINATION "${INCLUDE_SUBDIR}/${META_PROJECT_NAME}"
            COMPONENT header)
        if (NOT TARGET install-header)
            add_custom_target(install-header COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=header -P
                                                     "${CMAKE_BINARY_DIR}/cmake_install.cmake")
        endif ()
    endif ()

    # add install target for CMake modules
    foreach (CMAKE_MODULE_FILE ${CMAKE_MODULE_FILES})
        get_filename_component(CMAKE_MODULE_DIR ${CMAKE_MODULE_FILE} DIRECTORY)
        install(
            FILES ${CMAKE_MODULE_FILE}
            DESTINATION ${META_DATA_DIR}/${CMAKE_MODULE_DIR}
            COMPONENT cmake-modules)
    endforeach ()
    if (NOT TARGET install-cmake-modules)
        add_custom_target(install-cmake-modules COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=cmake-modules -P
                                                        "${CMAKE_BINARY_DIR}/cmake_install.cmake")
    endif ()

    # add install target for CMake templates
    foreach (CMAKE_TEMPLATE_FILE ${CMAKE_TEMPLATE_FILES})
        get_filename_component(CMAKE_TEMPLATE_DIR ${CMAKE_TEMPLATE_FILE} DIRECTORY)
        install(
            FILES ${CMAKE_TEMPLATE_FILE}
            DESTINATION ${META_DATA_DIR}/${CMAKE_TEMPLATE_DIR}
            COMPONENT cmake-templates)
    endforeach ()
    if (NOT TARGET install-cmake-templates)
        add_custom_target(install-cmake-templates COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=cmake-templates -P
                                                          "${CMAKE_BINARY_DIR}/cmake_install.cmake")
    endif ()

    # add install target for all the cmake stuff
    if (NOT TARGET install-cmake-stuff)
        add_custom_target(install-cmake-stuff)
        add_dependencies(install-cmake-stuff install-cmake-config install-cmake-modules install-cmake-templates)
    endif ()

    # add targets to ease creating mingw-w64 packages under Arch Linux
    if (MINGW)
        option(ENABLE_TARGETS_FOR_MINGW_CROSS_PACKAGING
               "enable targets to ease creating mingw-w64 packages under Arch Linux" OFF)
    else ()
        set(ENABLE_TARGETS_FOR_MINGW_CROSS_PACKAGING OFF)
    endif ()
    if (ENABLE_TARGETS_FOR_MINGW64_CROSS_PACKAGING)
        if (NOT TARGET install-mingw-w64)
            add_custom_target(install-mingw-w64)
        endif ()
        add_dependencies(install-mingw-w64 install-binary install-header install-cmake-stuff install-pkg-config)
        if (NOT TARGET install-mingw-w64-strip)
            add_custom_target(install-mingw-w64-strip)
        endif ()
        add_dependencies(install-mingw-w64-strip install-binary-strip install-header install-cmake-stuff install-pkg-config)
        if (LOCALIZATION_TARGET)
            add_dependencies(install-mingw-w64 ${LOCALIZATION_TARGET})
            add_dependencies(install-mingw-w64-strip ${LOCALIZATION_TARGET})
        endif ()
        find_program(STRIP_BINARY_PATH strip ONLY_CMAKE_FIND_ROOT_PATH)
        if (NOT STRIP_BINARY_PATH)
            message(FATAL_ERROR "Unable to find strip. Please set ${STRIP_BINARY_PATH}.")
        else ()
            message(STATUS "Using strip binary under \"${STRIP_BINARY_PATH}\".")
        endif ()
        if (NOT META_HEADER_ONLY_LIB)
            if (CMAKE_GENERATOR STREQUAL "Ninja")
                set(DESTDIR "\$\$\{DESTDIR\}\$\$\{DESTDIR:+/\}")
            else ()
                set(DESTDIR "\$\{DESTDIR\}\$\{DESTDIR:+/\}")
            endif ()
            add_custom_target(
                install-${META_TARGET_NAME}-mingw-w64-linker-file-strip
                COMMAND
                    "${STRIP_BINARY_PATH}" -g
                    "${DESTDIR}${CMAKE_INSTALL_FULL_LIBDIR}${SELECTED_LIB_SUFFIX}/$<TARGET_LINKER_FILE_NAME:${META_TARGET_NAME}>"
            )
            add_dependencies(install-${META_TARGET_NAME}-mingw-w64-linker-file-strip install-binary-strip)
            add_dependencies(install-mingw-w64-strip install-${META_TARGET_NAME}-mingw-w64-linker-file-strip)
        endif ()
    endif ()
endif ()

set(TARGET_CONFIG_DONE YES)
