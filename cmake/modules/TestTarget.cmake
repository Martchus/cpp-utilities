# before including this module, BasicConfig must be included

# always link test applications against c++utilities and cppunit
find_library(CPP_UNIT_LIB cppunit)

if(CPP_UNIT_LIB)
    list(APPEND TEST_LIBRARIES ${CPP_UTILITIES_LIB} ${CPP_UNIT_LIB})

    # set compile definitions
    if(NOT META_PUBLIC_SHARED_LIB_COMPILE_DEFINITIONS)
        set(META_PUBLIC_SHARED_LIB_COMPILE_DEFINITIONS ${META_PUBLIC_COMPILE_DEFINITIONS} ${META_ADDITIONAL_PUBLIC_SHARED_COMPILE_DEFINITIONS})
    endif()
    if(NOT META_PRIVATE_SHARED_LIB_COMPILE_DEFINITIONS)
        set(META_PRIVATE_SHARED_LIB_COMPILE_DEFINITIONS ${META_PRIVATE_COMPILE_DEFINITIONS} ${META_ADDITIONAL_PRIVATE_SHARED_COMPILE_DEFINITIONS})
    endif()

    # add autotools-style check target
    if(NOT TARGET check)
        set(CMAKE_CTEST_COMMAND ctest -V)
        add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})
        enable_testing()
    endif()

    # add test executable, but exclude it from the "all target"
    add_executable(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests EXCLUDE_FROM_ALL ${TEST_HEADER_FILES} ${TEST_SRC_FILES})

    # test applications of my projects always use c++utilities and cppunit
    if(NOT META_PROJECT_TYPE OR "${META_PROJECT_TYPE}" STREQUAL "library") # default project type is library
        # when testing a library, the test application always needs to link against it
        if(BUILD_SHARED_LIBS)
            list(APPEND TEST_LIBRARIES ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX})
            message(STATUS "Linking test target dynamically against ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}")
        else()
            list(APPEND TEST_LIBRARIES ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_static)
            message(STATUS "Linking test target statically against ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}")
        endif()
    endif()
    if("${META_PROJECT_TYPE}" STREQUAL "application")
        # the test application might need the path of the application to be tested
        set(APPLICATION_PATH "-a ${CMAKE_CURRENT_BINARY_DIR}/${META_PROJECT_NAME}")
        if(LINK_TESTS_AGAINST_APP_TARGET)
            # linking tests against the application target might be required
            # somehow this doesn't work when just specifying the applications target, so we need to specify the full path of the
            # target executable
            list(APPEND TEST_LIBRARIES "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}")
            add_dependencies(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX})
        endif()
    endif()
    target_link_libraries(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests
        PUBLIC ${ACTUAL_ADDITIONAL_LINK_FLAGS} "${PUBLIC_LIBRARIES}"
        PRIVATE "${TEST_LIBRARIES}" "${PRIVATE_LIBRARIES}"
    )
    target_compile_definitions(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests
        PUBLIC "${META_PUBLIC_SHARED_LIB_COMPILE_DEFINITIONS}"
        PRIVATE "${META_PRIVATE_SHARED_LIB_COMPILE_DEFINITIONS}"
    )
    set_target_properties(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests PROPERTIES
        CXX_STANDARD "${META_CXX_STANDARD}"
        LINK_SEARCH_START_STATIC ${STATIC_LINKAGE}
        LINK_SEARCH_END_STATIC ${STATIC_LINKAGE}
    )
    add_test(NAME ${META_PROJECT_NAME}_cppunit COMMAND ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests -p "${CMAKE_CURRENT_SOURCE_DIR}/testfiles" ${APPLICATION_PATH})

    # add the test executable to the dependencies of the check target
    add_dependencies(check ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests)

    # add target for launching tests with wine ensuring the WINEPATH is set correctly so wine is able to find all required *.dll files
    # requires script from c++utilities, hence the sources of c++utilities must be present
    if(MINGW AND CMAKE_CROSSCOMPILING AND CPP_UTILITIES_SOURCE_DIR)
        if(NOT TARGET ${META_PROJECT_NAME}_run_tests)
            if(CMAKE_FIND_ROOT_PATH)
                list(APPEND RUNTIME_LIBRARY_PATH "${CMAKE_FIND_ROOT_PATH}/bin")
            endif()
            add_custom_target(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_run_tests COMMAND "${CPP_UTILITIES_SOURCE_DIR}/scripts/wine.sh" "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests.${WINDOWS_EXT}" ${RUNTIME_LIBRARY_PATH})
            add_dependencies(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_run_tests ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests)
        endif()
    endif()

else()
    message(WARNING "Unable to add test target because cppunit could not be located.")

endif()
