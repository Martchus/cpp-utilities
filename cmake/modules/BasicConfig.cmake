# before including this module, the project meta-data must be set
if(NOT META_PROJECT_NAME)
    message(FATAL_ERROR "No project name (META_PROJECT_NAME) specified.")
endif()
if(NOT META_APP_NAME)
    message(FATAL_ERROR "No project name (META_APP_NAME) specified.")
endif()
if(NOT META_APP_AUTHOR)
    message(FATAL_ERROR "No project name (META_APP_AUTHOR) specified.")
endif()
if(NOT META_APP_DESCRIPTION)
    message(FATAL_ERROR "No project name (META_APP_DESCRIPTION) specified.")
endif()

# set project name (displayed in Qt Creator)
message(STATUS "Configuring project ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}")
project(${META_PROJECT_NAME})

# set META_PROJECT_VARNAME and META_PROJECT_VARNAME_UPPER if not specified explicitely
if(NOT META_PROJECT_VARNAME)
    set(META_PROJECT_VARNAME "${META_PROJECT_NAME}")
endif()
if(NOT META_PROJECT_VARNAME_UPPER)
    string(TOUPPER ${META_PROJECT_VARNAME} META_PROJECT_VARNAME_UPPER)
endif()
if(NOT META_PROJECT_VARNAME_LOWER)
    string(REGEX REPLACE "_+" "" META_PROJECT_VARNAME_LOWER "${META_PROJECT_VARNAME}")
    string(TOLOWER "${META_PROJECT_VARNAME_LOWER}" META_PROJECT_VARNAME_LOWER)
endif()

# set META_GENERIC_NAME to META_APP_NAME if not specified explicitely
if(NOT META_GENERIC_NAME)
    set(META_GENERIC_NAME "${META_APP_NAME}")
endif()

# set default CXX_STANDARD for all library, application and test targets
if(NOT META_CXX_STANDARD)
    set(META_CXX_STANDARD 14)
endif()

# set version to 0.0.0 if not specified explicitely
if(NOT META_VERSION_MAJOR)
    set(META_VERSION_MAJOR 0)
endif()
if(NOT META_VERSION_MINOR)
    set(META_VERSION_MINOR 0)
endif()
if(NOT META_VERSION_PATCH)
    set(META_VERSION_PATCH 0)
endif()

# set META_ID to META_PROJECT_NAME if not specified
if(NOT META_ID)
    set(META_ID "${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}")
endif()

# set bugtracker URL
if(NOT META_APP_BUGTRACKER_URL)
    if(META_APP_URL MATCHES "https://(github.com|gitlab.com|.*/gogs|gogs.*)/.*")
        set(META_APP_BUGTRACKER_URL "${META_APP_URL}/issues")
    else()
        set(META_APP_BUGTRACKER_URL "${META_APP_URL}")
    endif()
endif()

# determine license automatically from LICENSE file
if(NOT META_PROJECT_LICENSE)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
        file(READ "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE" META_PROJECT_LICENSE_FILE)
    elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE")
        file(READ "${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE" META_PROJECT_LICENSE_FILE)
    endif()
    if(META_PROJECT_LICENSE_FILE MATCHES "GNU GENERAL PUBLIC LICENSE.*Version ([1-9\\.]*)")
        set(META_PROJECT_LICENSE "GPL-${CMAKE_MATCH_1}")
    elseif(META_PROJECT_LICENSE_FILE MATCHES "GNU LESSER GENERAL PUBLIC LICENSE.*Version ([1-9\\.]*)")
        set(META_PROJECT_LICENSE "LGPL-${CMAKE_MATCH_1}")
    elseif(META_PROJECT_LICENSE_FILE MATCHES "MIT License")
        set(META_PROJECT_LICENSE "MIT")
    elseif(META_PROJECT_LICENSE_FILE MATCHES "Mozilla Public License Version ([1-9\\.]*)")
        set(META_PROJECT_LICENSE "MPL-${CMAKE_MATCH_1}")
    else()
        message(WARNING "Unable to detect license of ${META_PROJECT_NAME}. Set META_PROJECT_LICENSE manually to silence this warning.")
    endif()
endif()

# provide variables for other projects built as part of the same subdirs project
# to access files from this project
get_directory_property(HAS_PARENT PARENT_DIRECTORY)
if(HAS_PARENT)
    set(${META_PROJECT_VARNAME_UPPER}_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" PARENT_SCOPE)
    set(${META_PROJECT_VARNAME_UPPER}_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}" PARENT_SCOPE)
    set(${META_PROJECT_NAME}_DIR "${CMAKE_CURRENT_BINARY_DIR}" PARENT_SCOPE)
    if(CMAKE_FIND_ROOT_PATH AND MINGW)
        set(RUNTIME_LIBRARY_PATH "${CMAKE_CURRENT_BINARY_DIR}" ${RUNTIME_LIBRARY_PATH} PARENT_SCOPE)
    endif()
endif()

# determine version
set(META_APP_VERSION ${META_VERSION_MAJOR}.${META_VERSION_MINOR}.${META_VERSION_PATCH})
option(APPEND_GIT_REVISION "whether the build script should attempt to append the git revision and latest commit to the version displayed via --help" ON)
if(APPEND_GIT_REVISION)
    execute_process(
        COMMAND git rev-list --count HEAD
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE META_GIT_REV_COUNT
    )
    execute_process(
        COMMAND git rev-parse --short HEAD
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE META_GIT_LAST_COMMIT_ID
    )
    string(REPLACE "\n" "" META_GIT_REV_COUNT "${META_GIT_REV_COUNT}")
    string(REPLACE "\n" "" META_GIT_LAST_COMMIT_ID "${META_GIT_LAST_COMMIT_ID}")
    if(META_GIT_REV_COUNT AND META_GIT_LAST_COMMIT_ID)
        set(META_APP_VERSION ${META_APP_VERSION}-${META_GIT_REV_COUNT}.${META_GIT_LAST_COMMIT_ID})
    endif()
endif()

# set TARGET_EXECUTABLE which is used to refer to the target executable at its installation location
set(TARGET_EXECUTABLE "${CMAKE_INSTALL_PREFIX}/bin/${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}")

# create header for feature detection
if (META_FEATURES_FOR_COMPILER_DETECTION_HEADER)
    include(WriteCompilerDetectionHeader)
    write_compiler_detection_header(
      FILE "${CMAKE_CURRENT_BINARY_DIR}/resources/features.h"
      PREFIX "${META_PROJECT_VARNAME_UPPER}"
      COMPILERS GNU Clang
      FEATURES ${META_FEATURES_FOR_COMPILER_DETECTION_HEADER}
    )
endif()

# disable new ABI (can't catch ios_base::failure with new ABI)
option(FORCE_OLD_ABI "specifies whether usage of old ABI should be forced" OFF)
if(FORCE_OLD_ABI)
    list(APPEND META_PRIVATE_COMPILE_DEFINITIONS _GLIBCXX_USE_CXX11_ABI=0)
    message(STATUS "Forcing usage of old CXX11 ABI.")
else()
    message(STATUS "Using default CXX11 ABI (not forcing old CX11 ABI).")
endif()

# enable debug-only code when doing a debug build
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND META_PRIVATE_COMPILE_DEFINITIONS DEBUG_BUILD)
    message(STATUS "Debug build enabled.")
endif()

# enable logging when option is set
option(LOGGING_ENABLED "specifies whether logging is enabled" OFF)
if(LOGGING_ENABLED)
    list(APPEND META_PRIVATE_COMPILE_DEFINITIONS LOGGING_ENABLED)
    message(STATUS "Logging is enabled.")
endif()

# options for deciding whether to build static and/or shared libraries
if(("${META_PROJECT_TYPE}" STREQUAL "library") OR ("${META_PROJECT_TYPE}" STREQUAL "plugin") OR ("${META_PROJECT_TYPE}" STREQUAL "qtplugin") OR ("${META_PROJECT_TYPE}" STREQUAL ""))
    option(ENABLE_STATIC_LIBS "whether building static libraries is enabled (disabled by default)" OFF)
    option(DISABLE_SHARED_LIBS "whether building dynamic libraries is disabled (enabled by default)" OFF)
    if(DISABLE_SHARED_LIBS)
        set(BUILD_SHARED_LIBS OFF)
    else()
        set(BUILD_SHARED_LIBS ON)
    endif()
    if(ENABLE_STATIC_LIBS)
        set(BUILD_STATIC_LIBS ON)
    else()
        set(BUILD_STATIC_LIBS OFF)
    endif()
endif()

# options for forcing static linkage when building applications or dynamic libraries
if(("${META_PROJECT_TYPE}" STREQUAL "library") OR ("${META_PROJECT_TYPE}" STREQUAL "plugin") OR ("${META_PROJECT_TYPE}" STREQUAL "qtplugin") OR ("${META_PROJECT_TYPE}" STREQUAL ""))
    option(STATIC_LIBRARY_LINKAGE "forces static linkage when building dynamic libraries" OFF)
elseif("${META_PROJECT_TYPE}" STREQUAL "application")
    option(STATIC_LINKAGE "forces static linkage when building applications" OFF)
endif()

# additional linker flags used when static linkage is enables
if(NOT APPLE)
    list(APPEND META_ADDITIONAL_STATIC_LINK_FLAGS -static)
endif()
list(APPEND META_ADDITIONAL_STATIC_LINK_FLAGS -static-libstdc++ -static-libgcc)

# options for enabling/disabling Qt GUI (if available)
if(WIDGETS_HEADER_FILES OR WIDGETS_SRC_FILES OR WIDGETS_UI_FILES OR META_HAS_WIDGETS_GUI)
    if(META_GUI_OPTIONAL)
        option(WIDGETS_GUI "enables/disables building the Qt Widgets GUI: yes (default) or no" ON)
    else()
        set(WIDGETS_GUI ON)
    endif()
else()
    set(WIDGETS_GUI OFF)
endif()
if(QML_HEADER_FILES OR QML_SRC_FILES OR META_HAS_QUICK_GUI)
    if(META_GUI_OPTIONAL)
        option(QUICK_GUI "enables/disables building the Qt Quick GUI: yes (default) or no" ON)
    else()
        set(QUICK_GUI ON)
    endif()
else()
    set(QUICK_GUI OFF)
endif()

# find coding style (use style from c++utilities if none included in own project dir)
if(NOT META_NO_TIDY)
    set(CLANG_FORMAT_RULES "${CMAKE_CURRENT_SOURCE_DIR}/coding-style.clang-format")
    if(CPP_UTILITIES_SOURCE_DIR AND NOT EXISTS "${CLANG_FORMAT_RULES}")
        set(CLANG_FORMAT_RULES "${CPP_UTILITIES_SOURCE_DIR}/coding-style.clang-format")
    endif()
    if(NOT EXISTS "${CLANG_FORMAT_RULES}")
        set(CLANG_FORMAT_RULES "${CPP_UTILITIES_CONFIG_DIRS}/codingstyle.clang-format")
    endif()
endif()

# enable testing
enable_testing()
get_directory_property(HAS_PARENT PARENT_DIRECTORY)
if(HAS_PARENT)
    message(STATUS "For the check target to work, it is required to call enable_testing() on the source directory root.")
endif()

# make testing more convenient (not only useful if there's a test target; this is for instance also used in mocked configuration of syncthingtray)
# -> add a file called "srcdirref" to the build directory; this file contains the path of the sources so tests can easily find test files contained in the source directory
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/srcdirref" "${CMAKE_CURRENT_SOURCE_DIR}")
# -> ensure the directory "testfiles" exists in the build directory; tests will create the directory for working copies of testfiles there by default
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/testfiles")

# add target for tidying with clang-format
if(NOT META_NO_TIDY AND EXISTS "${CLANG_FORMAT_RULES}")
    option(CLANG_FORMAT_ENABLED "enables creation of tidy target using clang-format" OFF)
    if(CLANG_FORMAT_ENABLED)
        find_program(CLANG_FORMAT_BIN clang-format)
        if(CLANG_FORMAT_BIN)
            set(FORMATABLE_FILES
                ${HEADER_FILES} ${SRC_FILES}
                ${TEST_HEADER_FILES} ${TEST_SRC_FILES}
                ${GUI_HEADER_FILES} ${GUI_SRC_FILES}
                ${WIDGETS_HEADER_FILES} ${WIDGETS_SRC_FILES}
                ${QML_HEADER_FILES} ${QML_SRC_FILES}
            )
            if(FORMATABLE_FILES)
                # only tidy C/C++ files (and not eg. QML files)
                list(FILTER FORMATABLE_FILES INCLUDE REGEX ".*\\.(cpp|h)")

                 add_custom_command(
                    OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/.clang-format"
                    COMMAND "${CMAKE_COMMAND}" -E create_symlink "${CLANG_FORMAT_RULES}" "${CMAKE_CURRENT_SOURCE_DIR}/.clang-format"
                    COMMENT "Linking coding style from ${CLANG_FORMAT_RULES}"
                )
                add_custom_target("${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tidy"
                    COMMAND "${CLANG_FORMAT_BIN}" -style=file -i ${FORMATABLE_FILES}
                    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                    COMMENT "Tidying ${META_PROJECT_NAME} sources using clang-format"
                    DEPENDS "${FORMATABLE_FILES};${CMAKE_CURRENT_SOURCE_DIR}/.clang-format"
                )
                if(NOT TARGET tidy)
                    add_custom_target(tidy)
                endif()
                add_dependencies(tidy "${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tidy")

                # also add a test to verify whether sources are tidy
                add_test(NAME "${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tidy_test"
                    COMMAND "${CLANG_FORMAT_BIN}" -output-replacements-xml -style=file ${FORMATABLE_FILES}
                    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                )
                list(APPEND CHECK_TARGET_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/.clang-format")
                set_tests_properties("${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tidy_test" PROPERTIES
                    FAIL_REGULAR_EXPRESSION "<replacement.*>.*</replacement>"
                    REQUIRED_FILES "${CMAKE_CURRENT_SOURCE_DIR}/.clang-format"
                )
            endif()
        else()
            message(FATAL_ERROR "Unable to add tidy target; clang-format not found")
        endif()
    endif()
endif()

# add autotools-style check target
if(NOT TARGET check)
    set(CMAKE_CTEST_COMMAND ${CMAKE_CTEST_COMMAND} -V)
    add_custom_target(check
        COMMAND ${CMAKE_CTEST_COMMAND}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS "${CHECK_TARGET_DEPENDS}"
    )
endif()

# enable source code based coverage analysis using clang
option(CLANG_SOURCE_BASED_COVERAGE_ENABLED "enables creation of coverage targets for source-based coverage with clang" OFF)
if(CLANG_SOURCE_BASED_COVERAGE_ENABLED)
    if(NOT CMAKE_HOST_UNIX OR NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        message(FATAL_ERROR "Source-based coverage only available under UNIX with Clang")
    endif()
    if(NOT META_PROJECT_TYPE STREQUAL "application" AND DISABLE_SHARED_LIBS)
        message(FATAL_ERROR "Source-based coverage not available when only building static libs")
    endif()
    set(CLANG_SOURCE_BASED_COVERAGE_AVAILABLE YES)
    set(CLANG_SOURCE_BASED_COVERAGE_FLAGS -fprofile-instr-generate -fcoverage-mapping)
    list(APPEND META_PRIVATE_SHARED_LIB_COMPILE_OPTIONS ${CLANG_SOURCE_BASED_COVERAGE_FLAGS})
    list(APPEND META_PRIVATE_STATIC_LIB_COMPILE_OPTIONS ${CLANG_SOURCE_BASED_COVERAGE_FLAGS})
    list(APPEND META_ADDITIONAL_SHARED_LINK_FLAGS ${CLANG_SOURCE_BASED_COVERAGE_FLAGS})
    list(APPEND META_ADDITIONAL_STATIC_LINK_FLAGS ${CLANG_SOURCE_BASED_COVERAGE_FLAGS})
endif()

# configure creation of install targets
if(NOT META_NO_INSTALL_TARGETS)
    # install targets have not been disabled on project level
    # check whether install targets are disabled by the user
    # this might be useful since install targets seem to cause problems under MacOS
    option(ENABLE_INSTALL_TARGETS "enables creation of install targets" ON)
endif()

# add install target for extra files
if(NOT META_NO_INSTALL_TARGETS AND ENABLE_INSTALL_TARGETS)
    foreach(EXTRA_FILE ${EXTRA_FILES})
        get_filename_component(EXTRA_DIR ${EXTRA_FILE} DIRECTORY)
        install(
            FILES ${EXTRA_FILE}
            DESTINATION "share/${META_PROJECT_NAME}/${EXTRA_DIR}"
            COMPONENT extra-files
        )
    endforeach()
    if(NOT TARGET install-extra-files)
        add_custom_target(install-extra-files
            COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=extra-files -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
        )
    endif()
endif()

set(BASIC_PROJECT_CONFIG_DONE YES)
