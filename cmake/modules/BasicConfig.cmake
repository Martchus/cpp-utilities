cmake_minimum_required(VERSION 3.3.0 FATAL_ERROR)

# check whether the required project meta-data has been set before including this module
if (NOT META_PROJECT_NAME)
    message(FATAL_ERROR "No project name (META_PROJECT_NAME) specified.")
endif ()
if (NOT META_APP_NAME)
    message(FATAL_ERROR "No project name (META_APP_NAME) specified.")
endif ()
if (NOT META_APP_AUTHOR)
    message(FATAL_ERROR "No project name (META_APP_AUTHOR) specified.")
endif ()
if (NOT META_APP_DESCRIPTION)
    message(FATAL_ERROR "No project name (META_APP_DESCRIPTION) specified.")
endif ()

# set project name (displayed in Qt Creator) note: The project name is at least shown in Qt Creator this way but
# unfortunately setting project() from an included file is not sufficient (see
# https://cmake.org/cmake/help/latest/command/project.html#usage).
message(STATUS "Configuring project ${META_PROJECT_NAME}")
project(${META_PROJECT_NAME})

# set META_PROJECT_VARNAME and META_PROJECT_VARNAME_UPPER if not specified explicitely
if (NOT META_PROJECT_VARNAME)
    set(META_PROJECT_VARNAME "${META_PROJECT_NAME}")
endif ()
if (NOT META_PROJECT_VARNAME_UPPER)
    string(TOUPPER ${META_PROJECT_VARNAME} META_PROJECT_VARNAME_UPPER)
endif ()
if (NOT META_PROJECT_VARNAME_LOWER)
    string(REGEX REPLACE "_+" "" META_PROJECT_VARNAME_LOWER "${META_PROJECT_VARNAME}")
    string(TOLOWER "${META_PROJECT_VARNAME_LOWER}" META_PROJECT_VARNAME_LOWER)
endif ()

# allow setting a configuration name to allow installing multiple differently configured versions within the same prefix
# (intended to be used for installing Qt 5 and Qt 6 version or shared and static version within the same prefix)
set(${META_PROJECT_VARNAME_UPPER}_CONFIGURATION_NAME
    ""
    CACHE STRING "sets the configuration name for ${META_PROJECT_NAME}")
set(CONFIGURATION_NAME
    ""
    CACHE STRING "sets the configuration name for all projects within the current build")
set(CONFIGURATION_DISPLAY_NAME
    "${CONFIGURATION_NAME}"
    CACHE STRING
          "sets the display name for the configuration; incorporated in META_APP_NAME; defaults to CONFIGURATION_NAME")
if (${META_PROJECT_VARNAME_UPPER}_CONFIGURATION_NAME STREQUAL "none")
    set(META_CONFIG_NAME "")
elseif (${META_PROJECT_VARNAME_UPPER}_CONFIGURATION_NAME)
    set(META_CONFIG_NAME "${${META_PROJECT_VARNAME_UPPER}_CONFIGURATION_NAME}")
else ()
    set(META_CONFIG_NAME "${CONFIGURATION_NAME}")
endif ()
if (META_CONFIG_NAME)
    set(META_CONFIG_PREFIX "${META_CONFIG_NAME}-")
    set(META_CONFIG_SUFFIX "-${META_CONFIG_NAME}")
endif ()
if (CONFIGURATION_DISPLAY_NAME)
    set(META_APP_NAME "${META_APP_NAME} (${CONFIGURATION_DISPLAY_NAME})")
endif ()

# allow setting a library/application target suffix - A different configuration name might not require a different target
# name since it might differ anyways (e.g. library extensions for static and shared configuration). Hence there's not simply
# the configuration name used to distinguish targets as well.
if (NOT DEFINED ${META_PROJECT_VARNAME_UPPER}_CONFIGURATION_TARGET_SUFFIX)
    # wrap this within "if (NOT DEFINED" so absence of a target suffix can be enforced within certain project files
    set(${META_PROJECT_VARNAME_UPPER}_CONFIGURATION_TARGET_SUFFIX
        ""
        CACHE STRING "sets a target suffix for ${META_PROJECT_NAME}")
endif ()
set(CONFIGURATION_TARGET_SUFFIX
    ""
    CACHE STRING "sets the target suffix for all projects within the current build")
if (${META_PROJECT_VARNAME_UPPER}_CONFIGURATION_TARGET_SUFFIX STREQUAL "none")
    set(TARGET_SUFFIX "")
elseif (${META_PROJECT_VARNAME_UPPER}_CONFIGURATION_TARGET_SUFFIX)
    set(TARGET_SUFFIX "-${${META_PROJECT_VARNAME_UPPER}_CONFIGURATION_TARGET_SUFFIX}")
elseif (CONFIGURATION_TARGET_SUFFIX)
    set(TARGET_SUFFIX "-${CONFIGURATION_TARGET_SUFFIX}")
endif ()

# disable linking against default Qt plugins by default
if (NOT DEFINED META_QT_DEFAULT_PLUGINS)
    # note: The CMake modules in qtutilities take care of linking against static plugins on their own because old Qt versions
    # did not provide any support and one had to do it manually. Considering that with Qt's default my projects end up
    # pulling in needlessly many plugins I prefer to keep my current approach and disable Qt's defaults.
    set(META_QT_DEFAULT_PLUGINS 0) # needs to be exactly 0, Qt's code uses STREQUAL 0
endif ()

# find standard installation directories - note: Allow overriding CMAKE_INSTALL_LIBDIR and LIB_INSTALL_DIR but don't use the
# default from GNUInstallDirs (as an Arch Linux user this feels odd and I also want to avoid breaking existing build
# scripts).
set(LIB_INSTALL_DIR
    "lib"
    CACHE STRING "sets the directory to install libraries to (within the prefix)")
set(CMAKE_INSTALL_LIBDIR
    "${LIB_INSTALL_DIR}"
    CACHE STRING "sets the directory to install libraries to (within the prefix)")
include(GNUInstallDirs)

# define a few variables
set(META_TARGET_NAME "${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}")
set(META_DATA_DIR "${CMAKE_INSTALL_DATAROOTDIR}/${META_PROJECT_NAME}${META_CONFIG_SUFFIX}")
set(META_DATA_DIR_ABSOLUTE "${CMAKE_INSTALL_FULL_DATAROOTDIR}/${META_PROJECT_NAME}${META_CONFIG_SUFFIX}")
string(TOUPPER "${CMAKE_BUILD_TYPE}" META_CURRENT_CONFIGURATION)

# set META_GENERIC_NAME to META_APP_NAME if not specified explicitely
if (NOT META_GENERIC_NAME)
    set(META_GENERIC_NAME "${META_APP_NAME}")
endif ()

# set default CXX_STANDARD for all library, application and test targets
if (NOT META_CXX_STANDARD)
    set(META_CXX_STANDARD 17)
endif ()

# set version to 0.0.0 if not specified explicitely
if (NOT META_VERSION_MAJOR)
    set(META_VERSION_MAJOR 0)
endif ()
if (NOT META_VERSION_MINOR)
    set(META_VERSION_MINOR 0)
endif ()
if (NOT META_VERSION_PATCH)
    set(META_VERSION_PATCH 0)
endif ()

# set META_ID to target name if not specified
if (NOT META_ID)
    set(META_ID "${META_TARGET_NAME}")
endif ()

# set bugtracker URL
if (NOT META_APP_BUGTRACKER_URL)
    if (META_APP_URL MATCHES "https://(github.com|gitlab.com|.*/(gogs|gitea)|(gogs|gitea).*)/.*")
        set(META_APP_BUGTRACKER_URL "${META_APP_URL}/issues")
    else ()
        set(META_APP_BUGTRACKER_URL "${META_APP_URL}")
    endif ()
endif ()

# determine license automatically from LICENSE file
if (NOT META_PROJECT_LICENSE)
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
        file(READ "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE" META_PROJECT_LICENSE_FILE)
    elseif (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE")
        file(READ "${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE" META_PROJECT_LICENSE_FILE)
    elseif (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../LICENSE")
        file(READ "${CMAKE_CURRENT_SOURCE_DIR}/../../LICENSE" META_PROJECT_LICENSE_FILE)
    endif ()
    if (META_PROJECT_LICENSE_FILE MATCHES "GNU GENERAL PUBLIC LICENSE.*Version ([1-9\\.]*)")
        set(META_PROJECT_LICENSE "GPL-${CMAKE_MATCH_1}")
    elseif (META_PROJECT_LICENSE_FILE MATCHES "GNU LESSER GENERAL PUBLIC LICENSE.*Version ([1-9\\.]*)")
        set(META_PROJECT_LICENSE "LGPL-${CMAKE_MATCH_1}")
    elseif (META_PROJECT_LICENSE_FILE MATCHES "MIT License")
        set(META_PROJECT_LICENSE "MIT")
    elseif (META_PROJECT_LICENSE_FILE MATCHES "Mozilla Public License Version ([1-9\\.]*)")
        set(META_PROJECT_LICENSE "MPL-${CMAKE_MATCH_1}")
    else ()
        message(
            WARNING
                "Unable to detect license of ${META_PROJECT_NAME}. Set META_PROJECT_LICENSE manually to silence this warning."
        )
    endif ()
endif ()

# provide variables for other projects built as part of the same subdirs project to access files from this project
get_directory_property(HAS_PARENT PARENT_DIRECTORY)
if (HAS_PARENT)
    set(${META_PROJECT_VARNAME_UPPER}_SOURCE_DIR
        "${CMAKE_CURRENT_SOURCE_DIR}"
        PARENT_SCOPE)
    set(${META_PROJECT_VARNAME_UPPER}_BINARY_DIR
        "${CMAKE_CURRENT_BINARY_DIR}"
        PARENT_SCOPE)
    set(${META_PROJECT_NAME}_DIR
        "${CMAKE_CURRENT_BINARY_DIR}"
        PARENT_SCOPE)
    set(RUNTIME_LIBRARY_PATH
        "${CMAKE_CURRENT_BINARY_DIR}" ${RUNTIME_LIBRARY_PATH}
        PARENT_SCOPE)
endif ()

# determine version
set(META_APP_VERSION ${META_VERSION_MAJOR}.${META_VERSION_MINOR}.${META_VERSION_PATCH})
option(
    APPEND_GIT_REVISION
    "whether the build script should attempt to append the Git revision and latest commit to the version displayed via --help"
    ON)
if (APPEND_GIT_REVISION AND (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git" OR EXISTS "${CMAKE_SOURCE_DIR}/.git"))
    find_program(GIT_BIN git)
    execute_process(
        COMMAND ${GIT_BIN} rev-list --count HEAD
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE META_GIT_REV_COUNT)
    execute_process(
        COMMAND ${GIT_BIN} rev-parse --short HEAD
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE META_GIT_LAST_COMMIT_ID)
    string(REPLACE "\n" "" META_GIT_REV_COUNT "${META_GIT_REV_COUNT}")
    string(REPLACE "\n" "" META_GIT_LAST_COMMIT_ID "${META_GIT_LAST_COMMIT_ID}")
    if (META_GIT_REV_COUNT AND META_GIT_LAST_COMMIT_ID)
        set(META_APP_VERSION ${META_APP_VERSION}-${META_GIT_REV_COUNT}.${META_GIT_LAST_COMMIT_ID})
    endif ()
endif ()

# set TARGET_EXECUTABLE which is used to refer to the target executable at its installation location
set(TARGET_EXECUTABLE "${CMAKE_INSTALL_FULL_BINDIR}/${META_TARGET_NAME}")

# create header for feature detection
if (META_FEATURES_FOR_COMPILER_DETECTION_HEADER)
    include(WriteCompilerDetectionHeader)
    write_compiler_detection_header(
        FILE "${CMAKE_CURRENT_BINARY_DIR}/resources/features.h" PREFIX "${META_PROJECT_VARNAME_UPPER}"
        COMPILERS GNU Clang AppleClang
        FEATURES ${META_FEATURES_FOR_COMPILER_DETECTION_HEADER})
endif ()

# disable new ABI (can't catch ios_base::failure with new ABI)
option(FORCE_OLD_ABI "specifies whether usage of old ABI should be forced" OFF)
if (FORCE_OLD_ABI)
    list(APPEND META_PRIVATE_COMPILE_DEFINITIONS _GLIBCXX_USE_CXX11_ABI=0)
    message(STATUS "Forcing usage of old CXX11 ABI.")
else ()
    message(STATUS "Using default CXX11 ABI (not forcing old CX11 ABI).")
endif ()

# enable debug-only code when doing a debug build
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND META_PRIVATE_COMPILE_DEFINITIONS CPP_UTILITIES_DEBUG_BUILD)
    message(STATUS "Debug build enabled.")
endif ()

# enable logging when option is set
option(LOGGING_ENABLED "specifies whether logging is enabled" OFF)
if (LOGGING_ENABLED)
    list(APPEND META_PRIVATE_COMPILE_DEFINITIONS LOGGING_ENABLED)
    message(STATUS "Logging is enabled.")
endif ()

# determine whether the project is a header-only library
if (SRC_FILES
    OR GUI_SRC_FILES
    OR WIDGETS_SRC_FILES
    OR WIDGETS_UI_FILES
    OR QML_SRC_FILES
    OR RES_FILES)
    set(META_HEADER_ONLY_LIB NO)
else ()
    set(META_HEADER_ONLY_LIB YES)
    if ("${META_PROJECT_TYPE}" STREQUAL "application")
        message(FATAL_ERROR "Project ${META_PROJECT_NAME} is supposed to be an application but has only header files.")
    endif ()
    message(STATUS "Project ${META_PROJECT_NAME} is header-only library.")
endif ()

# ensure 3rdParty has been included to configure BUILD_SHARED_LIBS and STATIC_LINKAGE/STATIC_LIBRARY_LINKAGE
include(3rdParty)

# options for enabling/disabling Qt GUI (if available)
if (WIDGETS_HEADER_FILES
    OR WIDGETS_SRC_FILES
    OR WIDGETS_UI_FILES
    OR META_HAS_WIDGETS_GUI)
    if (META_GUI_OPTIONAL)
        option(WIDGETS_GUI "enables/disables building the Qt Widgets GUI: yes (default) or no" ON)
    else ()
        set(WIDGETS_GUI ON)
    endif ()
else ()
    set(WIDGETS_GUI OFF)
endif ()
if (QML_HEADER_FILES
    OR QML_SRC_FILES
    OR META_HAS_QUICK_GUI)
    if (META_GUI_OPTIONAL)
        option(QUICK_GUI "enables/disables building the Qt Quick GUI: yes (default) or no" ON)
    else ()
        set(QUICK_GUI ON)
    endif ()
else ()
    set(QUICK_GUI OFF)
endif ()

# find coding style (use style from c++utilities if none included in own project dir)
if (NOT META_NO_TIDY)
    set(CLANG_FORMAT_RULES "${CMAKE_CURRENT_SOURCE_DIR}/coding-style.clang-format")
    if (CPP_UTILITIES_SOURCE_DIR AND NOT EXISTS "${CLANG_FORMAT_RULES}")
        set(CLANG_FORMAT_RULES "${CPP_UTILITIES_SOURCE_DIR}/coding-style.clang-format")
    endif ()
    if (NOT EXISTS "${CLANG_FORMAT_RULES}")
        set(CLANG_FORMAT_RULES "${CPP_UTILITIES_CONFIG_DIRS}/codingstyle.clang-format")
    endif ()
endif ()

# enable testing
enable_testing()
get_directory_property(HAS_PARENT PARENT_DIRECTORY)
if (HAS_PARENT)
    message(STATUS "For the check target to work, it is required to call enable_testing() on the source directory root.")
endif ()

# make finding testfiles in out-of-source-tree build more convenient by adding a reference to the source directory (not only
# useful if there's a test target; this is for instance also used in mocked configuration of syncthingtray) -> add a file
# called "srcdirref" to the build directory; this file contains the path of the sources so tests can easily find test files
# contained in the source directory
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/srcdirref" "${CMAKE_CURRENT_SOURCE_DIR}")
# -> ensure the directory "testfiles" exists in the build directory; tests of my projects use it by default to create working
# copies of testfiles
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/testfiles")

# determine source files which might be passed to clang-format or clang-tidy
set(FORMATABLE_FILES
    ${HEADER_FILES}
    ${SRC_FILES}
    ${TEST_HEADER_FILES}
    ${TEST_SRC_FILES}
    ${GUI_HEADER_FILES}
    ${GUI_SRC_FILES}
    ${WIDGETS_HEADER_FILES}
    ${WIDGETS_SRC_FILES}
    ${QML_HEADER_FILES}
    ${QML_SRC_FILES}
    ${EXCLUDED_FILES})
# only format C/C++ files (and not eg. QML files)
if (FORMATABLE_FILES)
    list(FILTER FORMATABLE_FILES INCLUDE REGEX ".*\\.(c|cpp|h|hpp)")
endif ()

# determine source files which might be passed to cmake-format
set(FORMATABLE_FILES_CMAKE ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt ${CMAKE_MODULE_FILES})

# add command for symlinking clang-{format,tidy} rules so the tools can find it
if (EXISTS "${CLANG_FORMAT_RULES}")
    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/.clang-format"
        COMMAND "${CMAKE_COMMAND}" -E create_symlink "${CLANG_FORMAT_RULES}" "${CMAKE_CURRENT_SOURCE_DIR}/.clang-format"
        COMMENT "Linking coding style from ${CLANG_FORMAT_RULES}")
else ()
    message(WARNING "Format rules for clang-format not found.")
endif ()

# allow user to configure creation of tidy targets unless the project disables this via META_NO_TIDY
if (NOT META_NO_TIDY)
    option(CLANG_FORMAT_ENABLED "enables creation of tidy target using clang-format" OFF)
    option(CMAKE_FORMAT_ENABLED "enables creation of tidy target using cmake-format" OFF)
endif ()

# add target for tidying with clang-format
if (NOT META_NO_TIDY
    AND CLANG_FORMAT_ENABLED
    AND FORMATABLE_FILES
    AND EXISTS "${CLANG_FORMAT_RULES}")
    find_program(CLANG_FORMAT_BIN clang-format)
    if (NOT CLANG_FORMAT_BIN)
        message(FATAL_ERROR "Unable to add tidy target; clang-format not found")
    endif ()
    add_custom_target(
        "${META_TARGET_NAME}_tidy"
        COMMAND "${CLANG_FORMAT_BIN}" -style=file -i ${FORMATABLE_FILES}
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Tidying ${META_PROJECT_NAME} sources using clang-format"
        DEPENDS "${FORMATABLE_FILES};${CMAKE_CURRENT_SOURCE_DIR}/.clang-format")
    if (NOT TARGET tidy)
        add_custom_target(tidy)
    endif ()
    add_dependencies(tidy "${META_TARGET_NAME}_tidy")

    # also add a test to verify whether sources are tidy
    add_test(
        NAME "${META_TARGET_NAME}_tidy_test"
        COMMAND "${CLANG_FORMAT_BIN}" -output-replacements-xml -style=file ${FORMATABLE_FILES}
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
    list(APPEND CHECK_TARGET_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/.clang-format")
    set_tests_properties(
        "${META_TARGET_NAME}_tidy_test" PROPERTIES FAIL_REGULAR_EXPRESSION "<replacement.*>.*</replacement>" REQUIRED_FILES
                                                   "${CMAKE_CURRENT_SOURCE_DIR}/.clang-format")
endif ()

# add target for tidying with cmake-format
if (NOT META_NO_TIDY
    AND CMAKE_FORMAT_ENABLED
    AND FORMATABLE_FILES_CMAKE)
    find_program(CMAKE_FORMAT_BIN cmake-format)
    if (NOT CMAKE_FORMAT_BIN)
        message(FATAL_ERROR "Unable to add tidy target; cmake-format not found")
    endif ()
    if (NOT META_CMAKE_FORMAT_OPTIONS)
        set(META_CMAKE_FORMAT_OPTIONS --tab-size=4 --separate-ctrl-name-with-space=True --line-width=125 --autosort=False)
    endif ()
    set(CMAKE_FORMAT_COMMANDS)
    foreach (FILE_TO_FORMAT ${FORMATABLE_FILES_CMAKE})
        list(
            APPEND
            CMAKE_FORMAT_COMMANDS
            COMMAND
            "${CMAKE_FORMAT_BIN}"
            --in-place
            ${META_CMAKE_FORMAT_OPTIONS}
            "${FILE_TO_FORMAT}")
    endforeach ()
    add_custom_target(
        "${META_TARGET_NAME}_cmake_tidy"
        ${CMAKE_FORMAT_COMMANDS}
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Tidying ${META_PROJECT_NAME} sources using cmake-format"
        DEPENDS "${FORMATABLE_FILES_CMAKE}")
    if (NOT TARGET tidy)
        add_custom_target(tidy)
    endif ()
    add_dependencies(tidy "${META_TARGET_NAME}_cmake_tidy")
endif ()

# add target for static code analysis using clang-tidy
if (NOT META_NO_STATIC_ANALYSIS AND FORMATABLE_FILES)
    option(CLANG_TIDY_ENABLED "enables creation of static-check target using clang-tidy" OFF)
    set(CLANG_TIDY_CHECKS
        ""
        CACHE STRING
              "-*,clang-analyzer-*,cppcoreguidelines-*,modernize-*,performance-*,portability-*,readability-*,android-*")
    if (CLANG_TIDY_ENABLED)
        find_program(CLANG_TIDY_BIN clang-tidy)
        if (NOT CLANG_TIDY_BIN)
            message(FATAL_ERROR "Unable to add tidy target; clang-tidy not found")
        endif ()

        set(CLANG_TIDY_DEPENDS ${FORMATABLE_FILES})

        # compose options for clang-tidy
        set(CLANG_TIDY_OPTIONS -checks="${CLANG_TIDY_CHECKS}" -header-filter="^${META_PROJECT_NAME}/")
        if (EXISTS "${CLANG_FORMAT_RULES}")
            list(APPEND CLANG_TIDY_OPTIONS "-format-style=file")
            list(APPEND CLANG_TIDY_DPENDS "${CMAKE_CURRENT_SOURCE_DIR}/.clang-format")
        endif ()

        # compose CXX flags for clang-tidy
        set(CLANG_TIDY_CXX_FLAGS "")
        if (NOT META_HEADER_ONLY_LIB)
            # deduce flags from target, set c++ standard
            list(APPEND CLANG_TIDY_CXX_FLAGS "-std=c++$<TARGET_PROPERTY:${META_TARGET_NAME},CXX_STANDARD>")
            # add compile flags
            set(PROP "$<TARGET_PROPERTY:${META_TARGET_NAME},COMPILE_FLAGS>")
            list(APPEND CLANG_TIDY_CXX_FLAGS "$<$<BOOL:${PROP}>:$<JOIN:${PROP},$<SEMICOLON>>>")
            # add compile definitions
            set(PROP "$<TARGET_PROPERTY:${META_TARGET_NAME},COMPILE_DEFINITIONS>")
            list(APPEND CLANG_TIDY_CXX_FLAGS "$<$<BOOL:${PROP}>:-D$<JOIN:${PROP},$<SEMICOLON>-D>>")
            # add include directories
            set(PROP "$<TARGET_PROPERTY:${META_TARGET_NAME},INCLUDE_DIRECTORIES>")
            list(APPEND CLANG_TIDY_CXX_FLAGS "$<$<BOOL:${PROP}>:-I$<JOIN:${PROP},$<SEMICOLON>-I>>")
        else ()
            # set at least c++ standard for header-only libs
            list(APPEND CLANG_TIDY_CXX_FLAGS "-std=c++${META_CXX_STANDARD}")
        endif ()

        # add a custom command for each source file
        set(CLANG_TIDY_SYMBOLIC_OUTPUT_FILES "")
        foreach (FILE ${FORMATABLE_FILES})
            # skip header files
            if (${FILE} MATCHES ".*\.h")
                continue()
            endif ()

            # use symbolic output file since there's no actual output file (we're just interested in the log)
            set(SYMBOLIC_OUTPUT_FILE "${FILE}.clang-tidy-output")
            list(APPEND CLANG_TIDY_SYMBOLIC_OUTPUT_FILES "${SYMBOLIC_OUTPUT_FILE}")

            add_custom_command(
                OUTPUT "${SYMBOLIC_OUTPUT_FILE}"
                COMMAND "${CLANG_TIDY_BIN}" ${FILE} -- ${CLANG_TIDY_CXX_FLAGS}
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                COMMENT "Linting ${FILE} using clang-tidy"
                DEPENDS "${FILE}"
                COMMAND_EXPAND_LISTS VERBATIM)
        endforeach ()

        # mark all symbolic output files actually as symbolic
        set_source_files_properties(${CLANG_TIDY_SYMBOLIC_OUTPUT_FILES} PROPERTIES SYMBOLIC YES)

        # add targets
        add_custom_target(
            "${META_TARGET_NAME}_static_check"
            DEPENDS ${CLANG_TIDY_SYMBOLIC_OUTPUT_FILES}
            COMMENT "Linting ${META_TARGET_NAME} sources using clang-tidy")
        if (NOT TARGET static-check)
            add_custom_target(static-check)
        endif ()
        add_dependencies(static-check "${META_TARGET_NAME}_static_check")
    endif ()
endif ()

# add autotools-style check target
if (NOT TARGET check)
    set(CMAKE_CTEST_COMMAND ${CMAKE_CTEST_COMMAND} -V)
    add_custom_target(
        check
        COMMAND ${CMAKE_CTEST_COMMAND}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS "${CHECK_TARGET_DEPENDS}")
endif ()

# enable source code based coverage analysis using clang
option(CLANG_SOURCE_BASED_COVERAGE_ENABLED "enables creation of coverage targets for source-based coverage with clang" OFF)
if (CLANG_SOURCE_BASED_COVERAGE_ENABLED)
    if (NOT CMAKE_HOST_UNIX OR NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        message(FATAL_ERROR "Source-based coverage only available under UNIX with Clang")
    endif ()
    if (NOT META_PROJECT_TYPE STREQUAL "application" AND DISABLE_SHARED_LIBS)
        message(FATAL_ERROR "Source-based coverage not available when only building static libs")
    endif ()
    set(CLANG_SOURCE_BASED_COVERAGE_AVAILABLE YES)
    set(CLANG_SOURCE_BASED_COVERAGE_FLAGS -fprofile-instr-generate -fcoverage-mapping)
    list(APPEND META_PRIVATE_COMPILE_OPTIONS ${CLANG_SOURCE_BASED_COVERAGE_FLAGS})
    list(APPEND META_ADDITIONAL_LINK_FLAGS ${CLANG_SOURCE_BASED_COVERAGE_FLAGS})
endif ()

# configure creation of install targets
if (NOT META_NO_INSTALL_TARGETS)
    # install targets have not been disabled on project level check whether install targets are disabled by the user this
    # might be useful since install targets seem to cause problems under MacOS
    option(ENABLE_INSTALL_TARGETS "enables creation of install targets" ON)
endif ()

# add install target for extra files
if (NOT META_NO_INSTALL_TARGETS AND ENABLE_INSTALL_TARGETS)
    foreach (EXTRA_FILE ${EXTRA_FILES})
        get_filename_component(EXTRA_DIR ${EXTRA_FILE} DIRECTORY)
        install(
            FILES ${EXTRA_FILE}
            DESTINATION "${META_DATA_DIR}/${EXTRA_DIR}"
            COMPONENT extra-files)
    endforeach ()
    if (NOT TARGET install-extra-files)
        add_custom_target(install-extra-files COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=extra-files -P
                                                      "${CMAKE_BINARY_DIR}/cmake_install.cmake")
    endif ()
endif ()

# determine library directory suffix - Applications might be built as libraries under some platforms (eg. Android). Hence
# this is part of BasicConfig and not LibraryConfig.
set(LIB_SUFFIX
    ""
    CACHE STRING "specifies the general suffix for the library directory")
set(SELECTED_LIB_SUFFIX "${LIB_SUFFIX}")
set(LIB_SUFFIX_32
    ""
    CACHE STRING "specifies the suffix for the library directory to be used when building 32-bit library")
set(LIB_SUFFIX_64
    ""
    CACHE STRING "specifies the suffix for the library directory to be used when building 64-bit library")
if (LIB_SUFFIX_64 AND CMAKE_SIZEOF_VOID_P MATCHES "8")
    set(SELECTED_LIB_SUFFIX "${LIB_SUFFIX_64}")
elseif (LIB_SUFFIX_32 AND CMAKE_SIZEOF_VOID_P MATCHES "4")
    set(SELECTED_LIB_SUFFIX "${LIB_SUFFIX_32}")
endif ()

# ignore LIB_SUFFIX variables if CMAKE_INSTALL_LIBDIR ends with that suffix anyways (%cmake RPM macro apparently passes
# LIB_SUFFIX and CMAKE_INSTALL_LIBDIR/LIB_INSTALL_DIR at the same time)
if (CMAKE_INSTALL_LIBDIR MATCHES ".*${SELECTED_LIB_SUFFIX}$")
    set(SELECTED_LIB_SUFFIX "")
endif ()

set(BIN_INSTALL_DESTINATION "${CMAKE_INSTALL_FULL_BINDIR}")
set(LIB_INSTALL_DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}${SELECTED_LIB_SUFFIX}")

# allow user to specify additional libraries to link against (see buildvariables.md for details)
set(USER_DEFINED_ADDITIONAL_LIBRARIES
    ""
    CACHE STRING "specifies additional libraries to link against (added after any other libraries to the linker line)")
function (append_user_defined_additional_libraries)
    if (NOT USER_DEFINED_ADDITIONAL_LIBRARIES)
        return()
    endif ()

    # find the last library
    set(LIBS PRIVATE_LIBRARIES)
    list(LENGTH ${LIBS} LIB_COUNT)
    if (LIB_COUNT LESS_EQUAL 0)
        set(LIBS PUBLIC_LIBRARIES)
        list(LENGTH ${LIBS} LIB_COUNT)
    endif ()
    if (LIB_COUNT LESS_EQUAL 0)
        # just add the addiitional libs to PRIVATE_LIBRARIES if there are no libs yet anyways
        set(PRIVATE_LIBRARIES
            "${USER_DEFINED_ADDITIONAL_LIBRARIES}"
            PARENT_SCOPE)
    endif ()
    math(EXPR LAST_LIB_INDEX "${LIB_COUNT} - 1")
    list(GET ${LIBS} ${LAST_LIB_INDEX} LAST_LIB)

    # add the additional libs as INTERFACE_LINK_LIBRARIES of the last lib if it is a target
    if (TARGET "${LAST_LIB}")
        # note: Otherwise the INTERFACE_LINK_LIBRARIES of the last target might still come after the
        # USER_DEFINED_ADDITIONAL_LIBRARIES on the linker line.
        set_property(
            TARGET "${LAST_LIB}"
            APPEND
            PROPERTY INTERFACE_LINK_LIBRARIES ${USER_DEFINED_ADDITIONAL_LIBRARIES})

        return()
    endif ()

    # fall back to simply append the library to PRIVATE_LIBRARIES
    set(PRIVATE_LIBRARIES
        "${USER_DEFINED_ADDITIONAL_LIBRARIES}"
        PARENT_SCOPE)
endfunction ()

# locate PNG icon which used for generating icons for the Windows executable and the macOS bundle
if (PNG_ICON_PATH)
    if (NOT EXISTS "${PNG_ICON_PATH}")
        message(FATAL_ERROR "The specified PNG_ICON_PATH \"${PNG_ICON_PATH}\" is invalid.")
    endif ()
else ()
    if (PNG_ICON_SIZE)
        set(PNG_ICON_SIZES_TO_TEST "${PNG_ICON_SIZE}")
    else ()
        set(PNG_ICON_SIZES_TO_TEST 256 128 64 32 16)
    endif ()
    foreach (POSSIBLE_PNG_ICON_SIZE ${PNG_ICON_SIZES_TO_TEST})
        set(PNG_ICON_PATH
            "${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/hicolor/${POSSIBLE_PNG_ICON_SIZE}x${POSSIBLE_PNG_ICON_SIZE}/apps/${META_PROJECT_NAME}.png"
        )
        if (EXISTS "${PNG_ICON_PATH}")
            set(PNG_ICON_SIZE "${POSSIBLE_PNG_ICON_SIZE}")
            message(STATUS "Using PNG icon from \"${PNG_ICON_PATH}\".")
            break()
        endif ()
        unset(PNG_ICON_PATH)
    endforeach ()
endif ()

set(BASIC_PROJECT_CONFIG_DONE YES)
