cmake_minimum_required(VERSION 3.3.0 FATAL_ERROR)

if (NOT BASIC_PROJECT_CONFIG_DONE)
    message(FATAL_ERROR "Before including the TestTarget module, the BasicConfig module must be included.")
endif ()
if (TEST_CONFIG_DONE)
    message(FATAL_ERROR "Can not include TestTarget module when tests are already configured.")
endif ()

include(TestUtilities)

# find and link against cppunit if required (used by all my projects, so it is required by default)
if (NOT META_NO_CPP_UNIT)
    # make cppunit library/include dir configurable
    set(CPP_UNIT_LIB
        NOTFOUND
        CACHE FILEPATH "cppunit lib" FORCE)
    set(CPP_UNIT_INCLUDE_DIR
        NOTFOUND
        CACHE FILEPATH "cppunit include dir" FORCE)

    # set default for minimum version (only checked when using pkg-config)
    if (NOT META_REQUIRED_CPP_UNIT_VERSION)
        set(META_REQUIRED_CPP_UNIT_VERSION 1.13.0)
    endif ()

    # auto-detection: try to find via pkg-config first
    if (NOT CPP_UNIT_LIB AND NOT CPP_UNIT_INCLUDE_DIR)
        include(FindPkgConfig)
        pkg_search_module(CPP_UNIT_CONFIG_${META_PROJECT_NAME} cppunit>=${META_REQUIRED_CPP_UNIT_VERSION})
        if (CPP_UNIT_CONFIG_${META_PROJECT_NAME}_FOUND)
            set(CPP_UNIT_LIB
                "${CPP_UNIT_CONFIG_${META_PROJECT_NAME}_LDFLAGS_OTHER}" "${CPP_UNIT_CONFIG_${META_PROJECT_NAME}_LIBRARIES}"
                CACHE FILEPATH "cppunit lib" FORCE)
            set(CPP_UNIT_INCLUDE_DIR
                ${CPP_UNIT_CONFIG_${META_PROJECT_NAME}_INCLUDE_DIRS}
                CACHE FILEPATH "cppunit include dir" FORCE)
            link_directories(${CPP_UNIT_CONFIG_${META_PROJECT_NAME}_LIBRARY_DIRS})
        else ()
            # fall back to find_library
            find_library(DETECTED_CPP_UNIT_LIB cppunit)
            set(CPP_UNIT_LIB
                "${DETECTED_CPP_UNIT_LIB}"
                CACHE FILEPATH "cppunit lib" FORCE)
        endif ()
    endif ()

    if (NOT CPP_UNIT_LIB)
        message(WARNING "Unable to add test target because cppunit could not be located.")
        set(META_HAVE_TESTS NO)
        set(TEST_CONFIG_DONE YES)
        return()
    endif ()

    list(APPEND TEST_LIBRARIES "${CPP_UNIT_LIB}")
    if (NOT CPP_UNIT_CONFIG_${META_PROJECT_NAME}_FOUND)
        message(
            WARNING
                "Cppunit not detected via pkg-config so the version couldn't be checked. Required version for ${META_PROJECT_NAME} is ${META_REQUIRED_CPP_UNIT_VERSION}."
        )
    endif ()

    if (CPP_UNIT_INCLUDE_DIR)
        list(APPEND TEST_INCLUDE_DIRS "${CPP_UNIT_INCLUDE_DIR}")
    endif ()
endif ()

# add default cppunit test application if requested
if (META_ADD_DEFAULT_CPP_UNIT_TEST_APPLICATION)
    if (META_NO_CPP_UNIT)
        message(
            FATAL_ERROR
                "Project ${META_PROJECT_NAME} has META_ADD_DEFAULT_CPP_UNIT_TEST_APPLICATION and META_NO_CPP_UNIT enabled at the same time."
        )
    endif ()

    set(DEFAULT_CPP_UNIT_TEST_APPLICATION_SRC "${CMAKE_CURRENT_BINARY_DIR}/cppunit.cpp")
    file(WRITE "${DEFAULT_CPP_UNIT_TEST_APPLICATION_SRC}" "#include <c++utilities/tests/cppunit.h>")
    list(APPEND TEST_SRC_FILES "${DEFAULT_CPP_UNIT_TEST_APPLICATION_SRC}")
endif ()

# always link test applications against c++utilities
list(APPEND TEST_LIBRARIES ${CPP_UTILITIES_LIB})

# handle testing a library (which is default project type)
if (META_PROJECT_IS_LIBRARY)
    # when testing a library, the test application always needs to link against it
    list(APPEND TEST_LIBRARIES ${META_TARGET_NAME})
    message(STATUS "Linking test target against ${META_TARGET_NAME}")
endif ()

# create a 'testlib' and link tests against it when testing an application an the tests need to call internal functions of
# the application
if (META_PROJECT_IS_APPLICATION AND LINK_TESTS_AGAINST_APP_TARGET)
    # create target for the 'testlib'
    set(TESTLIB_FILES ${HEADER_FILES} ${SRC_FILES} ${WIDGETS_FILES} ${QML_FILES} ${RES_FILES} ${QM_FILES})
    list(REMOVE_ITEM TESTLIB_FILES main.h main.cpp)
    add_library(${META_TARGET_NAME}_testlib SHARED ${TESTLIB_FILES})
    target_link_libraries(
        ${META_TARGET_NAME}_testlib
        PUBLIC ${META_ADDITIONAL_LINK_FLAGS} "${PUBLIC_LIBRARIES}"
        PRIVATE "${PRIVATE_LIBRARIES}")
    target_include_directories(
        ${META_TARGET_NAME}_testlib
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> $<INSTALL_INTERFACE:${HEADER_INSTALL_DESTINATION}>
               ${PUBLIC_INCLUDE_DIRS}
        PRIVATE "${PRIVATE_INCLUDE_DIRS}")
    target_compile_definitions(
        ${META_TARGET_NAME}_testlib
        PUBLIC "${META_PUBLIC_COMPILE_DEFINITIONS}"
        PRIVATE "${META_PRIVATE_COMPILE_DEFINITIONS}")
    target_compile_options(
        ${META_TARGET_NAME}_testlib
        PUBLIC "${META_PUBLIC_COMPILE_OPTIONS}"
        PRIVATE "${META_PRIVATE_COMPILE_OPTIONS}")
    set_target_properties(
        ${META_TARGET_NAME}_testlib
        PROPERTIES CXX_STANDARD "${META_CXX_STANDARD}"
                   C_VISIBILITY_PRESET default
                   CXX_VISIBILITY_PRESET default
                   LINK_SEARCH_START_STATIC ${STATIC_LINKAGE}
                   LINK_SEARCH_END_STATIC ${STATIC_LINKAGE}
                   AUTOGEN_TARGET_DEPENDS "${AUTOGEN_DEPS}")
    if (CPP_UNIT_CONFIG_${META_PROJECT_NAME}_FOUND)
        target_include_directories(${META_TARGET_NAME}_testlib
                                   PRIVATE "${CPP_UNIT_CONFIG_${META_PROJECT_NAME}_INCLUDE_DIRS}")
        target_compile_options(${META_TARGET_NAME}_testlib PRIVATE "${CPP_UNIT_CONFIG_${META_PROJECT_NAME}_CFLAGS_OTHER}")
    endif ()
    # link tests against it
    list(APPEND TEST_LIBRARIES ${META_TARGET_NAME}_testlib)
    # ensure all symbols are visible (man gcc: "Despite the nomenclature, default always means public")
    set_target_properties(${META_TARGET_NAME}_testlib PROPERTIES CXX_VISIBILITY_PRESET default)
endif ()

# configure test target
unset(TEST_TARGET_OPTIONS)
unset(RUN_TESTS_APPLICATION_ARGS)
if (META_PROJECT_TYPE STREQUAL "application")
    set(RUN_TESTS_APPLICATION_ARGS -a "$<TARGET_FILE:${META_TARGET_NAME}_tests>")
endif ()
set(RUN_TESTS_ARGS -p "${CMAKE_CURRENT_SOURCE_DIR}/testfiles" -w "${CMAKE_CURRENT_BINARY_DIR}/testworkingdir"
                   ${RUN_TESTS_APPLICATION_ARGS})
if (META_TEST_TARGET_IS_MANUAL)
    list(APPEND TEST_TARGET_OPTIONS MANUAL)
endif ()
configure_test_target(
    TARGET_NAME
    "${META_TARGET_NAME}"
    TEST_NAME
    "tests"
    HEADER_FILES
    "${TEST_HEADER_FILES}"
    SRC_FILES
    "${TEST_SRC_FILES}"
    LIBRARIES
    "${TEST_LIBRARIES}"
    RUN_ARGS
    "${RUN_TESTS_ARGS}"
    ${TEST_TARGET_OPTIONS})

# enable source code based coverage analysis using clang
if (CLANG_SOURCE_BASED_COVERAGE_AVAILABLE)
    # define path of raw profile data
    set(LLVM_PROFILE_RAW_FILE "${CMAKE_CURRENT_BINARY_DIR}/${META_TARGET_NAME}_tests.profraw")
    # define path of list with additional raw profile data from fork processes spawned during tests
    set(LLVM_PROFILE_RAW_LIST_FILE "${CMAKE_CURRENT_BINARY_DIR}/${META_TARGET_NAME}_tests.profraw.list")
    # define path of merged profile data generated from raw profiling data
    set(LLVM_PROFILE_DATA_FILE "${CMAKE_CURRENT_BINARY_DIR}/${META_TARGET_NAME}_tests.profdata")
    # define paths of output files
    set(COVERAGE_REPORT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${META_TARGET_NAME}_tests_coverage.txt")
    set(COVERAGE_PER_FILE_REPORT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${META_TARGET_NAME}_tests_coverage_per_file.txt")
    set(COVERAGE_OVERALL_REPORT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${META_TARGET_NAME}_tests_coverage_overall.txt")
    set(COVERAGE_HTML_REPORT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${META_TARGET_NAME}_tests_coverage.html")
    set(COVERAGE_REPORT_FILES "${COVERAGE_REPORT_FILE}")

    # specify where to store raw clang profiling data via environment variable
    if (NOT META_TEST_TARGET_IS_MANUAL)
        set_tests_properties(
            ${META_TARGET_NAME}_run_tests
            PROPERTIES ENVIRONMENT
                       "LLVM_PROFILE_FILE=${LLVM_PROFILE_RAW_FILE};LLVM_PROFILE_LIST_FILE=${LLVM_PROFILE_RAW_LIST_FILE}")
    endif ()

    # add command to execute tests generating raw profiling data
    add_custom_command(
        OUTPUT "${LLVM_PROFILE_RAW_FILE}" "${LLVM_PROFILE_RAW_LIST_FILE}"
        COMMAND
            "${CMAKE_COMMAND}" -E env "LLVM_PROFILE_FILE=${LLVM_PROFILE_RAW_FILE}"
            "LLVM_PROFILE_LIST_FILE=${LLVM_PROFILE_RAW_LIST_FILE}" $<TARGET_FILE:${META_TARGET_NAME}_tests> -p
            "${CMAKE_CURRENT_SOURCE_DIR}/testfiles" -w "${CMAKE_CURRENT_BINARY_DIR}/testworkingdir"
            ${RUN_TESTS_APPLICATION_ARGS}
        COMMENT "Executing ${META_TARGET_NAME}_tests to generate raw profiling data for source-based coverage report"
        DEPENDS "${META_TARGET_NAME}_tests")

    # add commands for processing raw profiling data
    find_program(LLVM_PROFDATA_BIN llvm-profdata)
    find_program(LLVM_COV_BIN llvm-cov)
    if (LLVM_PROFDATA_BIN AND LLVM_COV_BIN)
        # merge profiling data
        add_custom_command(
            OUTPUT "${LLVM_PROFILE_DATA_FILE}"
            COMMAND cat "${LLVM_PROFILE_RAW_LIST_FILE}" | xargs "${LLVM_PROFDATA_BIN}" merge -o "${LLVM_PROFILE_DATA_FILE}"
                    -sparse "${LLVM_PROFILE_RAW_FILE}"
            COMMENT "Generating profiling data for source-based coverage report"
            DEPENDS "${LLVM_PROFILE_RAW_FILE}" "${LLVM_PROFILE_RAW_LIST_FILE}")

        # determine llvm-cov version
        execute_process(COMMAND "${LLVM_COV_BIN}" -version OUTPUT_VARIABLE LLVM_COV_VERSION)
        string(REGEX MATCH "LLVM version ([0-9](.[0-9])*)" LLVM_COV_VERSION "${LLVM_COV_VERSION}")
        if (CMAKE_MATCH_1)
            set(LLVM_COV_VERSION "${CMAKE_MATCH_1}")
        else ()
            message(
                FATAL_ERROR
                    "Unable to determine version of llvm-cov. Output of ${LLVM_COV_BIN} -version:\n${LLVM_COV_VERSION}")
        endif ()

        # determine the target file for llvm-cov
        if (NOT META_HEADER_ONLY_LIB)
            set(LLVM_COV_TARGET_FILE $<TARGET_FILE:${META_TARGET_NAME}> $<TARGET_FILE:${META_TARGET_NAME}_tests>)
        else ()
            set(LLVM_COV_TARGET_FILE $<TARGET_FILE:${META_TARGET_NAME}_tests>)
        endif ()

        # generate coverage report with statistics per function
        unset(LLVM_COV_ADDITIONAL_OPTIONS)
        if (LLVM_COV_VERSION GREATER_EQUAL 5.0.0)
            # LLVM 5 introduced -show-functions; this is required to get the same behavior as previously (statistics on
            # function-level)
            list(APPEND LLVM_COV_ADDITIONAL_OPTIONS -show-functions)
        endif ()
        add_custom_command(
            OUTPUT "${COVERAGE_REPORT_FILE}"
            COMMAND
                "${LLVM_COV_BIN}" report -format=text -stats -instr-profile "${LLVM_PROFILE_DATA_FILE}"
                ${LLVM_COV_ADDITIONAL_OPTIONS} ${LLVM_COV_TARGET_FILE} ${HEADER_FILES} ${SRC_FILES} ${WIDGETS_HEADER_FILES}
                ${WIDGETS_SOURCE_FILES} ${QML_HEADER_FILES} ${QML_SOURCE_FILES} > "${COVERAGE_REPORT_FILE}"
            COMMENT "Generating coverage report (statistics per function)"
            DEPENDS "${LLVM_PROFILE_DATA_FILE}"
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

        # generate coverage report with statistics per file (only possible with LLVM 5 if source files are specified)
        if (LLVM_COV_VERSION GREATER_EQUAL 5.0.0)
            add_custom_command(
                OUTPUT "${COVERAGE_PER_FILE_REPORT_FILE}"
                COMMAND
                    "${LLVM_COV_BIN}" report -format=text -stats -instr-profile "${LLVM_PROFILE_DATA_FILE}"
                    ${LLVM_COV_TARGET_FILE} ${HEADER_FILES} ${SRC_FILES} ${WIDGETS_HEADER_FILES} ${WIDGETS_SOURCE_FILES}
                    ${QML_HEADER_FILES} ${QML_SOURCE_FILES} > "${COVERAGE_PER_FILE_REPORT_FILE}"
                COMMENT "Generating coverage report (statistics per file)"
                DEPENDS "${LLVM_PROFILE_DATA_FILE}"
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
            list(APPEND COVERAGE_REPORT_FILES "${COVERAGE_PER_FILE_REPORT_FILE}")
        endif ()

        # add target for the coverage reports
        add_custom_target("${META_TARGET_NAME}_tests_coverage_summary" DEPENDS ${COVERAGE_REPORT_FILES})

        # NOTE: Those commands have been added before the release of LLVM 5 where coverage reports with statistics per file
        # could not be generated.

        # generate coverage overall report (total region/line coverage)
        set(OVERALL_COVERAGE_AKW_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tests/calculateoverallcoverage.awk")
        if (CPP_UTILITIES_SOURCE_DIR AND NOT EXISTS "${OVERALL_COVERAGE_AKW_SCRIPT}")
            set(OVERALL_COVERAGE_AKW_SCRIPT "${CPP_UTILITIES_SOURCE_DIR}/tests/calculateoverallcoverage.awk")
        endif ()
        if (NOT EXISTS "${OVERALL_COVERAGE_AKW_SCRIPT}")
            set(OVERALL_COVERAGE_AKW_SCRIPT "${CPP_UTILITIES_CONFIG_DIRS}/tests/calculateoverallcoverage.awk")
        endif ()
        add_custom_command(
            OUTPUT "${COVERAGE_OVERALL_REPORT_FILE}"
            COMMAND awk -f "${OVERALL_COVERAGE_AKW_SCRIPT}" "${COVERAGE_REPORT_FILE}" > "${COVERAGE_OVERALL_REPORT_FILE}"
            COMMENT "Generating coverage report (overall figures)"
            DEPENDS "${OVERALL_COVERAGE_AKW_SCRIPT}" "${COVERAGE_REPORT_FILE}")
        add_custom_target("${META_TARGET_NAME}_tests_coverage_overall_summary" DEPENDS "${COVERAGE_OVERALL_REPORT_FILE}")

        # generate HTML document showing covered/uncovered code
        add_custom_command(
            OUTPUT "${COVERAGE_HTML_REPORT_FILE}"
            COMMAND
                "${LLVM_COV_BIN}" show -project-title="${META_APP_NAME}" -format=html -instr-profile
                "${LLVM_PROFILE_DATA_FILE}" ${LLVM_COV_TARGET_FILE} ${HEADER_FILES} ${SRC_FILES} ${WIDGETS_FILES}
                ${QML_FILES} > "${COVERAGE_HTML_REPORT_FILE}"
            COMMENT "Generating HTML document showing covered/uncovered code"
            DEPENDS "${LLVM_PROFILE_DATA_FILE}"
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
        add_custom_target("${META_TARGET_NAME}_tests_coverage_html" DEPENDS "${COVERAGE_HTML_REPORT_FILE}")

        # create target for all coverage docs
        add_custom_target(
            "${META_TARGET_NAME}_tests_coverage"
            DEPENDS ${COVERAGE_REPORT_FILES}
            DEPENDS "${COVERAGE_OVERALL_REPORT_FILE}"
            DEPENDS "${COVERAGE_HTML_REPORT_FILE}")

        # add targets to global coverage target
        if (NOT TARGET coverage)
            add_custom_target(coverage)
        endif ()
        add_dependencies(coverage "${META_TARGET_NAME}_tests_coverage")
    else ()
        message(
            FATAL_ERROR "Unable to generate target for coverage report because llvm-profdata and llvm-cov are not available."
        )
    endif ()
endif ()

set(META_HAVE_TESTS YES)
set(TEST_CONFIG_DONE YES)
