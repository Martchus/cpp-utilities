if(NOT BASIC_PROJECT_CONFIG_DONE)
    message(FATAL_ERROR "Before including the TestTarget module, the BasicConfig module must be included.")
endif()
if(TEST_CONFIG_DONE)
    message(FATAL_ERROR "Can not include TestTarget module when tests are already configured.")
endif()

option(EXCLUDE_TESTS_FROM_ALL "specifies whether to exclude tests from the \"all\" target (enabled by default)" ON)

# find and link against cppunit if required (used by all my projects, so it is required by default)
if(NOT META_NO_CPP_UNIT)
    # make cppunit library/include dir configurable
    set(CPP_UNIT_LIB NOTFOUND CACHE FILEPATH "cppunit lib" FORCE)
    set(CPP_UNIT_INCLUDE_DIR NOTFOUND CACHE FILEPATH "cppunit include dir" FORCE)

    # set default for minimum version (only checked when using pkg-config)
    if(NOT META_REQUIRED_CPP_UNIT_VERSION)
        set(META_REQUIRED_CPP_UNIT_VERSION 1.13.0)
    endif()

    # auto-detection: try to find via pkg-config first
    if(NOT CPP_UNIT_LIB AND NOT CPP_UNIT_INCLUDE_DIR)
        include(FindPkgConfig)
        pkg_search_module(CPP_UNIT_CONFIG_${META_PROJECT_NAME} cppunit>=${META_REQUIRED_CPP_UNIT_VERSION})
        if(CPP_UNIT_CONFIG_${META_PROJECT_NAME}_FOUND)
            set(CPP_UNIT_LIB "${CPP_UNIT_CONFIG_${META_PROJECT_NAME}_LDFLAGS_OTHER}" "${CPP_UNIT_CONFIG_${META_PROJECT_NAME}_LIBRARIES}" CACHE FILEPATH "cppunit lib" FORCE)
            set(CPP_UNIT_INCLUDE_DIR ${CPP_UNIT_CONFIG_${META_PROJECT_NAME}_INCLUDE_DIRS} CACHE FILEPATH "cppunit include dir" FORCE)
            link_directories(${CPP_UNIT_CONFIG_${META_PROJECT_NAME}_LIBRARY_DIRS})
        else()
            # fall back to find_library
            find_library(DETECTED_CPP_UNIT_LIB cppunit)
            set(CPP_UNIT_LIB "${DETECTED_CPP_UNIT_LIB}" CACHE FILEPATH "cppunit lib" FORCE)
        endif()
    endif()

    if(CPP_UNIT_LIB)
        list(APPEND TEST_LIBRARIES "${CPP_UNIT_LIB}")
        if(NOT CPP_UNIT_CONFIG_${META_PROJECT_NAME}_FOUND)
            message(WARNING "Cppunit not detected via pkg-config so the version couldn't be checked. Required version for ${META_PROJECT_NAME} is ${META_REQUIRED_CPP_UNIT_VERSION}.")
        endif()
    endif()
    if(CPP_UNIT_INCLUDE_DIR)
        list(APPEND TEST_INCLUDE_DIRS "${CPP_UNIT_INCLUDE_DIR}")
    endif()
endif()

if(CPP_UNIT_LIB OR META_NO_CPP_UNIT)
    # always link test applications against c++utilities
    list(APPEND TEST_LIBRARIES ${CPP_UTILITIES_LIB})

    # set compile definitions
    if(NOT META_PUBLIC_SHARED_LIB_COMPILE_DEFINITIONS)
        set(META_PUBLIC_SHARED_LIB_COMPILE_DEFINITIONS ${META_PUBLIC_COMPILE_DEFINITIONS} ${META_ADDITIONAL_PUBLIC_SHARED_COMPILE_DEFINITIONS})
    endif()
    if(NOT META_PRIVATE_SHARED_LIB_COMPILE_DEFINITIONS)
        set(META_PRIVATE_SHARED_LIB_COMPILE_DEFINITIONS ${META_PRIVATE_COMPILE_DEFINITIONS} ${META_ADDITIONAL_PRIVATE_SHARED_COMPILE_DEFINITIONS})
    endif()

    # add target for test executable, but exclude it from the "all" target when EXCLUDE_TESTS_FROM_ALL is set
    if(EXCLUDE_TESTS_FROM_ALL)
        set(TESTS_EXCLUSION EXCLUDE_FROM_ALL)
    else()
        unset(TESTS_EXCLUSION)
    endif()
    add_executable(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests ${TESTS_EXCLUSION} ${TEST_HEADER_FILES} ${TEST_SRC_FILES})

    # add top-level target to build all test targets conveniently, also when excluded from "all" target
    if(NOT TARGET tests)
        add_custom_target(tests DEPENDS ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests)
    else()
        add_dependencies(tests ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests)
    endif()

    # handle testing a library (which is default project type)
    if(NOT META_PROJECT_TYPE OR "${META_PROJECT_TYPE}" STREQUAL "library")
        # when testing a library, the test application always needs to link against it
        if(BUILD_SHARED_LIBS)
            list(APPEND TEST_LIBRARIES ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX})
            message(STATUS "Linking test target dynamically against ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}")
        else()
            list(APPEND TEST_LIBRARIES ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_static)
            message(STATUS "Linking test target statically against ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}")
        endif()
    endif()

    # handle testing an application
    if("${META_PROJECT_TYPE}" STREQUAL "application")
        # using functions directly from the tests might be required -> also create a 'testlib' and link tests against it
        if(LINK_TESTS_AGAINST_APP_TARGET)
            # create target for the 'testlib'
            set(TESTLIB_FILES ${HEADER_FILES} ${SRC_FILES} ${WIDGETS_FILES} ${QML_FILES} ${RES_FILES} ${QM_FILES})
            list(REMOVE_ITEM TESTLIB_FILES main.h main.cpp)
            add_library(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_testlib SHARED ${TESTLIB_FILES})
            target_link_libraries(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_testlib
                PUBLIC ${ACTUAL_ADDITIONAL_LINK_FLAGS} "${PUBLIC_LIBRARIES}"
                PRIVATE "${PRIVATE_LIBRARIES}"
            )
            target_include_directories(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_testlib
                PUBLIC
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                    $<INSTALL_INTERFACE:${HEADER_INSTALL_DESTINATION}>
                    ${PUBLIC_SHARED_INCLUDE_DIRS}
                PRIVATE
                    "${PRIVATE_SHARED_INCLUDE_DIRS}"
            )
            target_compile_definitions(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_testlib
                PUBLIC "${META_PUBLIC_SHARED_LIB_COMPILE_DEFINITIONS}"
                PRIVATE "${META_PRIVATE_SHARED_LIB_COMPILE_DEFINITIONS}"
            )
            target_compile_options(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_testlib
                PUBLIC "${META_PUBLIC_SHARED_LIB_COMPILE_OPTIONS}"
                PRIVATE "${META_PRIVATE_SHARED_LIB_COMPILE_OPTIONS}"
            )
            set_target_properties(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_testlib PROPERTIES
                CXX_STANDARD "${META_CXX_STANDARD}"
                LINK_SEARCH_START_STATIC ${STATIC_LINKAGE}
                LINK_SEARCH_END_STATIC ${STATIC_LINKAGE}
                AUTOGEN_TARGET_DEPENDS "${AUTOGEN_DEPS}"
            )
            if(CPP_UNIT_CONFIG_${META_PROJECT_NAME}_FOUND)
                target_include_directories(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_testlib
                    PRIVATE "${CPP_UNIT_CONFIG_${META_PROJECT_NAME}_INCLUDE_DIRS}"
                )
                target_compile_options(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_testlib
                    PRIVATE "${CPP_UNIT_CONFIG_${META_PROJECT_NAME}_CFLAGS_OTHER}"
                )
            endif()
            # link tests against it
            list(APPEND TEST_LIBRARIES ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_testlib)
            # ensure all symbols are visible (man gcc: "Despite the nomenclature, default always means public")
            set_target_properties(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_testlib PROPERTIES CXX_VISIBILITY_PRESET default)
        endif()
    endif()

    # actually apply configuration for test target
    target_link_libraries(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests
        PUBLIC ${ACTUAL_ADDITIONAL_LINK_FLAGS} "${PUBLIC_LIBRARIES}"
        PRIVATE "${TEST_LIBRARIES}" "${PRIVATE_LIBRARIES}"
    )
    target_include_directories(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
            $<INSTALL_INTERFACE:${HEADER_INSTALL_DESTINATION}>
            ${PUBLIC_SHARED_INCLUDE_DIRS}
        PRIVATE
            ${TEST_INCLUDE_DIRS}
            "${PRIVATE_SHARED_INCLUDE_DIRS}"
    )
    target_compile_definitions(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests
        PUBLIC "${META_PUBLIC_SHARED_LIB_COMPILE_DEFINITIONS}"
        PRIVATE "${META_PRIVATE_SHARED_LIB_COMPILE_DEFINITIONS}"
    )
    target_compile_options(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests
        PUBLIC "${META_PUBLIC_SHARED_LIB_COMPILE_OPTIONS}"
        PRIVATE "${META_PRIVATE_SHARED_LIB_COMPILE_OPTIONS}"
    )
    set_target_properties(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests PROPERTIES
        CXX_STANDARD "${META_CXX_STANDARD}"
        LINK_SEARCH_START_STATIC ${STATIC_LINKAGE}
        LINK_SEARCH_END_STATIC ${STATIC_LINKAGE}
    )

    # make the test recognized by ctest
    unset(RUN_TESTS_APPLICATION_ARG)
    if(META_PROJECT_TYPE STREQUAL "application")
        set(RUN_TESTS_APPLICATION_ARGS -a "$<TARGET_FILE:${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}>")
    endif()
    if(NOT META_TEST_TARGET_IS_MANUAL)
        add_test(NAME ${META_PROJECT_NAME}_run_tests COMMAND
            ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests
            -p "${CMAKE_CURRENT_SOURCE_DIR}/testfiles"
            -w "${CMAKE_CURRENT_BINARY_DIR}/testworkingdir"
            ${RUN_TESTS_APPLICATION_ARGS}
        )
    endif()

    # enable source code based coverage analysis using clang
    if(CLANG_SOURCE_BASED_COVERAGE_AVAILABLE)
        # define path of raw profile data
        set(LLVM_PROFILE_RAW_FILE "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests.profraw")
        # define path of list with additional raw profile data from fork processes spawned during tests
        set(LLVM_PROFILE_RAW_LIST_FILE "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests.profraw.list")
        # define path of merged profile data generated from raw profiling data
        set(LLVM_PROFILE_DATA_FILE "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests.profdata")
        # define paths of output files
        set(COVERAGE_REPORT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests_coverage.txt")
        set(COVERAGE_PER_FILE_REPORT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests_coverage_per_file.txt")
        set(COVERAGE_OVERALL_REPORT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests_coverage_overall.txt")
        set(COVERAGE_HTML_REPORT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests_coverage.html")
        set(COVERAGE_REPORT_FILES "${COVERAGE_REPORT_FILE}")
        # specify where to store raw clang profiling data via environment variable
        if(NOT META_TEST_TARGET_IS_MANUAL)
            set_tests_properties(${META_PROJECT_NAME}_run_tests
                PROPERTIES ENVIRONMENT
                    "LLVM_PROFILE_FILE=${LLVM_PROFILE_RAW_FILE};LLVM_PROFILE_LIST_FILE=${LLVM_PROFILE_RAW_LIST_FILE}"
            )
        endif()
        add_custom_command(
            OUTPUT "${LLVM_PROFILE_RAW_FILE}"
                   "${LLVM_PROFILE_RAW_LIST_FILE}"
            COMMAND "${CMAKE_COMMAND}"
                    -E env
                        "LLVM_PROFILE_FILE=${LLVM_PROFILE_RAW_FILE}"
                        "LLVM_PROFILE_LIST_FILE=${LLVM_PROFILE_RAW_LIST_FILE}"
                    $<TARGET_FILE:${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests>
                        -p "${CMAKE_CURRENT_SOURCE_DIR}/testfiles"
                        -w "${CMAKE_CURRENT_BINARY_DIR}/testworkingdir"
                        ${RUN_TESTS_APPLICATION_ARGS}
            COMMENT "Executing ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests to generate raw profiling data for source-based coverage report"
            DEPENDS "${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests"
        )
        find_program(LLVM_PROFDATA_BIN llvm-profdata)
        find_program(LLVM_COV_BIN llvm-cov)
        if(LLVM_PROFDATA_BIN AND LLVM_COV_BIN)
            # merge profiling data
            add_custom_command(
                OUTPUT "${LLVM_PROFILE_DATA_FILE}"
                COMMAND cat "${LLVM_PROFILE_RAW_LIST_FILE}" | xargs
                        "${LLVM_PROFDATA_BIN}" merge
                        -o "${LLVM_PROFILE_DATA_FILE}"
                        -sparse
                        "${LLVM_PROFILE_RAW_FILE}"
                COMMENT "Generating profiling data for source-based coverage report"
                DEPENDS "${LLVM_PROFILE_RAW_FILE}"
                        "${LLVM_PROFILE_RAW_LIST_FILE}"
            )
            # determine llvm-cov version
            execute_process(
                COMMAND "${LLVM_COV_BIN}" -version
                OUTPUT_VARIABLE LLVM_COV_VERSION
            )
            string(REGEX MATCH "LLVM version ([0-9](.[0-9])*)" LLVM_COV_VERSION "${LLVM_COV_VERSION}")
            if(CMAKE_MATCH_1)
                set(LLVM_COV_VERSION "${CMAKE_MATCH_1}")
            else()
                message(FATAL_ERROR "Unable to determine version of llvm-cov. Output of ${LLVM_COV_BIN} -version:\n${LLVM_COV_VERSION}")
            endif()
            # determine the target file for llvm-cov
            if(NOT META_HEADER_ONLY_LIB)
                set(LLVM_COV_TARGET_FILE $<TARGET_FILE:${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}> $<TARGET_FILE:${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests>)
            else()
                set(LLVM_COV_TARGET_FILE $<TARGET_FILE:${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests>)
            endif()
            # generate coverage report with statistics per function
            unset(LLVM_COV_ADDITIONAL_OPTIONS)
            if(LLVM_COV_VERSION GREATER_EQUAL 5.0.0)
                # LLVM 5 introduced -show-functions; this is required to get the same behavior as previously (statistics on function-level)
                list(APPEND LLVM_COV_ADDITIONAL_OPTIONS -show-functions)
            endif()
            add_custom_command(
                OUTPUT "${COVERAGE_REPORT_FILE}"
                COMMAND "${LLVM_COV_BIN}" report
                    -format=text
                    -stats
                    -instr-profile "${LLVM_PROFILE_DATA_FILE}"
                    ${LLVM_COV_ADDITIONAL_OPTIONS}
                    ${LLVM_COV_TARGET_FILE}
                    ${HEADER_FILES} ${SRC_FILES} ${WIDGETS_HEADER_FILES} ${WIDGETS_SOURCE_FILES} ${QML_HEADER_FILES} ${QML_SOURCE_FILES}
                    > "${COVERAGE_REPORT_FILE}"
                COMMENT "Generating coverage report (statistics per function)"
                DEPENDS "${LLVM_PROFILE_DATA_FILE}"
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            )
            # generate coverage report with statistics per file (only possible with LLVM 5 if source files are specified)
            if(LLVM_COV_VERSION GREATER_EQUAL 5.0.0)
                add_custom_command(
                    OUTPUT "${COVERAGE_PER_FILE_REPORT_FILE}"
                    COMMAND "${LLVM_COV_BIN}" report
                        -format=text
                        -stats
                        -instr-profile "${LLVM_PROFILE_DATA_FILE}"
                        ${LLVM_COV_TARGET_FILE}
                        ${HEADER_FILES} ${SRC_FILES} ${WIDGETS_HEADER_FILES} ${WIDGETS_SOURCE_FILES} ${QML_HEADER_FILES} ${QML_SOURCE_FILES}
                        > "${COVERAGE_PER_FILE_REPORT_FILE}"
                    COMMENT "Generating coverage report (statistics per file)"
                    DEPENDS "${LLVM_PROFILE_DATA_FILE}"
                    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                )
                list(APPEND COVERAGE_REPORT_FILES "${COVERAGE_PER_FILE_REPORT_FILE}")
            endif()
            # add target for the coverage reports
            add_custom_target("${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests_coverage_summary"
                DEPENDS ${COVERAGE_REPORT_FILES}
            )
            # generate coverage overall report (total region/line coverage)
            # NOTE: added before release of LLVM 5 where coverage report with statistics per file could not be generated
            set(OVERALL_COVERAGE_AKW_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tests/calculateoverallcoverage.awk")
            if(CPP_UTILITIES_SOURCE_DIR AND NOT EXISTS "${OVERALL_COVERAGE_AKW_SCRIPT}")
                set(OVERALL_COVERAGE_AKW_SCRIPT "${CPP_UTILITIES_SOURCE_DIR}/tests/calculateoverallcoverage.awk")
            endif()
            if(NOT EXISTS "${OVERALL_COVERAGE_AKW_SCRIPT}")
                set(OVERALL_COVERAGE_AKW_SCRIPT "${CPP_UTILITIES_CONFIG_DIRS}/tests/calculateoverallcoverage.awk")
            endif()
            add_custom_command(
                OUTPUT "${COVERAGE_OVERALL_REPORT_FILE}"
                COMMAND awk
                    -f "${OVERALL_COVERAGE_AKW_SCRIPT}" "${COVERAGE_REPORT_FILE}"
                    > "${COVERAGE_OVERALL_REPORT_FILE}"
                COMMENT "Generating coverage report (overall figures)"
                DEPENDS "${OVERALL_COVERAGE_AKW_SCRIPT}" "${COVERAGE_REPORT_FILE}"
            )
            add_custom_target("${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests_coverage_overall_summary"
                DEPENDS "${COVERAGE_OVERALL_REPORT_FILE}"
            )
            # generate HTML document showing covered/uncovered code
            add_custom_command(
                OUTPUT "${COVERAGE_HTML_REPORT_FILE}"
                COMMAND "${LLVM_COV_BIN}" show
                    -project-title="${META_APP_NAME}"
                    -format=html
                    -instr-profile "${LLVM_PROFILE_DATA_FILE}"
                    ${LLVM_COV_TARGET_FILE}
                    ${HEADER_FILES} ${SRC_FILES} ${WIDGETS_FILES} ${QML_FILES}
                    > "${COVERAGE_HTML_REPORT_FILE}"
                COMMENT "Generating HTML document showing covered/uncovered code"
                DEPENDS "${LLVM_PROFILE_DATA_FILE}"
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            )
            add_custom_target("${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests_coverage_html"
                DEPENDS "${COVERAGE_HTML_REPORT_FILE}"
            )
            # create target for all coverage docs
            add_custom_target("${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests_coverage"
                DEPENDS ${COVERAGE_REPORT_FILES}
                DEPENDS "${COVERAGE_OVERALL_REPORT_FILE}"
                DEPENDS "${COVERAGE_HTML_REPORT_FILE}"
            )
            # add targets to global coverage target
            if(NOT TARGET coverage)
                add_custom_target(coverage)
            endif()
            add_dependencies(coverage "${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests_coverage")
        else()
            message(FATAL_ERROR "Unable to generate target for coverage report because llvm-profdata and llvm-cov are not available.")
        endif()
    endif()

    # add the test executable to the dependencies of the check target
    if(NOT META_TEST_TARGET_IS_MANUAL)
        add_dependencies(check ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests)
    endif()

    # add target for launching tests with wine ensuring the WINEPATH is set correctly so wine is able to find all required *.dll files
    # requires script from c++utilities, hence the sources of c++utilities must be present
    if(MINGW AND CMAKE_CROSSCOMPILING AND CPP_UTILITIES_SOURCE_DIR)
        if(NOT TARGET ${META_PROJECT_NAME}_run_tests_with_wine)
            if(CMAKE_FIND_ROOT_PATH)
                list(APPEND RUNTIME_LIBRARY_PATH "${CMAKE_FIND_ROOT_PATH}/bin")
            endif()
            add_custom_target(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_run_tests_with_wine COMMAND "${CPP_UTILITIES_SOURCE_DIR}/scripts/wine.sh" "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests.${WINDOWS_EXT}" ${RUNTIME_LIBRARY_PATH})
            add_dependencies(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_run_tests_with_wine ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests)
        endif()
    endif()

    set(META_HAVE_TESTS YES)

else()
    message(WARNING "Unable to add test target because cppunit could not be located.")
    set(META_HAVE_TESTS NO)
endif()

set(TEST_CONFIG_DONE YES)
