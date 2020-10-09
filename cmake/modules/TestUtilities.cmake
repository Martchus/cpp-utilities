cmake_minimum_required(VERSION 3.3.0 FATAL_ERROR)

# prevent multiple inclusion
if (DEFINED TESTING_UTILITIES_LOADED)
    return()
endif ()
set(TESTING_UTILITIES_LOADED YES)

option(EXCLUDE_TESTS_FROM_ALL "specifies whether to exclude tests from the 'all' target (enabled by default)" ON)

function (configure_test_target)
    # parse arguments
    set(OPTIONAL_ARGS MANUAL)
    set(ONE_VALUE_ARGS TARGET_NAME TEST_NAME)
    set(MULTI_VALUE_ARGS HEADER_FILES SRC_FILES LIBRARIES RUN_ARGS)
    cmake_parse_arguments(ARGS "${OPTIONAL_ARGS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" ${ARGN})
    if (NOT ARGS_TARGET_NAME)
        set(ARGS_TARGET_NAME "${META_TARGET_NAME}")
    endif ()
    if (NOT ARGS_TEST_NAME)
        message(FATAL_ERROR "No test name specified.")
    endif ()
    set(TEST_TARGET_NAME "${ARGS_TARGET_NAME}_${ARGS_TEST_NAME}")

    # add target for test executable, but exclude it from the "all" target when EXCLUDE_TESTS_FROM_ALL is set
    if (EXCLUDE_TESTS_FROM_ALL)
        set(TESTS_EXCLUSION EXCLUDE_FROM_ALL)
    else ()
        unset(TESTS_EXCLUSION)
    endif ()
    add_executable("${TEST_TARGET_NAME}" ${TESTS_EXCLUSION} ${ARGS_HEADER_FILES} ${ARGS_SRC_FILES})

    # add top-level target to build all test targets conveniently, also when excluded from "all" target
    if (NOT TARGET tests)
        add_custom_target(tests DEPENDS "${TEST_TARGET_NAME}")
    else ()
        add_dependencies(tests "${TEST_TARGET_NAME}")
    endif ()

    # configure test target
    target_link_libraries(
        "${TEST_TARGET_NAME}"
        PUBLIC ${META_ADDITIONAL_LINK_FLAGS} ${META_ADDITIONAL_LINK_FLAGS_TEST_TARGET} "${PUBLIC_LIBRARIES}"
        PRIVATE "${ARGS_LIBRARIES}" "${PRIVATE_LIBRARIES}")
    target_include_directories(
        "${TEST_TARGET_NAME}"
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> $<INSTALL_INTERFACE:${HEADER_INSTALL_DESTINATION}>
               ${PUBLIC_INCLUDE_DIRS}
        PRIVATE ${TEST_INCLUDE_DIRS} "${PRIVATE_INCLUDE_DIRS}")
    target_compile_definitions(
        "${TEST_TARGET_NAME}"
        PUBLIC "${META_PUBLIC_COMPILE_DEFINITIONS}"
        PRIVATE "${META_PRIVATE_COMPILE_DEFINITIONS}")
    target_compile_options(
        "${TEST_TARGET_NAME}"
        PUBLIC "${META_PUBLIC_COMPILE_OPTIONS}"
        PRIVATE "${META_PRIVATE_COMPILE_OPTIONS}")
    set_target_properties(
        "${TEST_TARGET_NAME}"
        PROPERTIES CXX_STANDARD "${META_CXX_STANDARD}"
                   C_VISIBILITY_PRESET hidden
                   CXX_VISIBILITY_PRESET hidden
                   LINK_SEARCH_START_STATIC ${STATIC_LINKAGE}
                   LINK_SEARCH_END_STATIC ${STATIC_LINKAGE})

    # make the test recognized by ctest
    if (NOT ARGS_MANUAL)
        add_test(NAME "${ARGS_TARGET_NAME}_run_${ARGS_TEST_NAME}" COMMAND "${TEST_TARGET_NAME}" ${RUN_ARGS})
    endif ()

    # add the test executable to the dependencies of the check target
    if (NOT ARGS_MANUAL AND TARGET check)
        add_dependencies(check ${TEST_TARGET_NAME})
    endif ()
endfunction ()
