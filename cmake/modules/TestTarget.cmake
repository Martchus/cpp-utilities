if(NOT BASIC_PROJECT_CONFIG_DONE)
    message(FATAL_ERROR "Before including the TestTarget module, the BasicConfig module must be included.")
endif()
if(TEST_CONFIG_DONE)
    message(FATAL_ERROR "Can not include TestTarget module when tests are already configured.")
endif()

option(EXCLUDE_TESTS_FROM_ALL "specifies whether to exclude tests from the \"all\" target (enabled by default)" ON)

# find cppunit (I always use it for tests of my libs/applications but maybe this should be made optional in the future)
find_library(CPP_UNIT_LIB cppunit)

if(CPP_UNIT_LIB)
    # always link test applications against c++utilities and cppunit
    list(APPEND TEST_LIBRARIES ${CPP_UTILITIES_LIB} ${CPP_UNIT_LIB})

    # set compile definitions
    if(NOT META_PUBLIC_SHARED_LIB_COMPILE_DEFINITIONS)
        set(META_PUBLIC_SHARED_LIB_COMPILE_DEFINITIONS ${META_PUBLIC_COMPILE_DEFINITIONS} ${META_ADDITIONAL_PUBLIC_SHARED_COMPILE_DEFINITIONS})
    endif()
    if(NOT META_PRIVATE_SHARED_LIB_COMPILE_DEFINITIONS)
        set(META_PRIVATE_SHARED_LIB_COMPILE_DEFINITIONS ${META_PRIVATE_COMPILE_DEFINITIONS} ${META_ADDITIONAL_PRIVATE_SHARED_COMPILE_DEFINITIONS})
    endif()

    # add autotools-style check target and enable testing
    if(NOT TARGET check)
        set(CMAKE_CTEST_COMMAND ctest -V)
        add_custom_target(check
            COMMAND ${CMAKE_CTEST_COMMAND}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
    endif()

    # enable testing
    enable_testing()
    get_directory_property(HAS_PARENT PARENT_DIRECTORY)
    if(HAS_PARENT)
        message(STATUS "For the check target to work, it is required to call enable_testing() on the source directory root.")
    endif()

    # add target for test executable, but exclude it from the "all target" when EXCLUDE_TESTS_FROM_ALL is set
    if(EXCLUDE_TESTS_FROM_ALL)
        set(TESTS_EXCLUSION EXCLUDE_FROM_ALL)
    else()
        unset(TESTS_EXCLUSION)
    endif()
    add_executable(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests ${TESTS_EXCLUSION} ${TEST_HEADER_FILES} ${TEST_SRC_FILES})

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
        # the test application might need the path of the application to be tested
        set(APPLICATION_PATH "-a ${CMAKE_CURRENT_BINARY_DIR}/${META_PROJECT_NAME}")

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
            target_compile_definitions(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_testlib
                PUBLIC "${META_PUBLIC_SHARED_LIB_COMPILE_DEFINITIONS}"
                PRIVATE "${META_PRIVATE_SHARED_LIB_COMPILE_DEFINITIONS}"
            )
            set_target_properties(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_testlib PROPERTIES
                CXX_STANDARD "${META_CXX_STANDARD}"
                LINK_SEARCH_START_STATIC ${STATIC_LINKAGE}
                LINK_SEARCH_END_STATIC ${STATIC_LINKAGE}
                AUTOGEN_TARGET_DEPENDS "${AUTOGEN_DEPS}"
            )
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
    target_compile_definitions(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests
        PUBLIC "${META_PUBLIC_SHARED_LIB_COMPILE_DEFINITIONS}"
        PRIVATE "${META_PRIVATE_SHARED_LIB_COMPILE_DEFINITIONS}"
    )
    set_target_properties(${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests PROPERTIES
        CXX_STANDARD "${META_CXX_STANDARD}"
        LINK_SEARCH_START_STATIC ${STATIC_LINKAGE}
        LINK_SEARCH_END_STATIC ${STATIC_LINKAGE}
    )
    add_test(NAME ${META_PROJECT_NAME}_run_tests COMMAND
        ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests
        -p "${CMAKE_CURRENT_SOURCE_DIR}/testfiles"
        -w "${CMAKE_CURRENT_BINARY_DIR}/testworkingdir"
        ${APPLICATION_PATH}
    )

    # add the test executable to the dependencies of the check target
    add_dependencies(check ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_tests)

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

else()
    message(WARNING "Unable to add test target because cppunit could not be located.")
endif()

set(TEST_CONFIG_DONE YES)
