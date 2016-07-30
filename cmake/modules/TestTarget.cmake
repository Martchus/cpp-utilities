# before including this module, BasicConfig must be included

# add autotools-style check target
if(NOT TARGET check)
    set(CMAKE_CTEST_COMMAND ctest -V)
    add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})
    enable_testing()
endif()

# add test executable, but exclude it from the "all target"
add_executable(${META_PROJECT_NAME}_tests EXCLUDE_FROM_ALL ${TEST_HEADER_FILES} ${TEST_SRC_FILES})

# always link test applications against c++utilities, cppunit and pthreads
find_library(CPP_UNIT_LIB cppunit)
find_library(PTHREAD_LIB pthread)
list(APPEND TEST_LIBRARIES ${CPP_UTILITIES_SHARED_LIB} ${CPP_UNIT_LIB} ${PTHREAD_LIB})

# test applications of my projects always use c++utilities and cppunit
if(NOT META_PROJECT_TYPE OR "${META_PROJECT_TYPE}" STREQUAL "library") # default project type is library
    # when testing a library, the test application always needs to link against it
    list(APPEND TEST_LIBRARIES ${META_PROJECT_NAME})
else()
    # otherwise, the tests application needs the path of the application to be tested
    set(APPLICATION_PATH "-a ${CMAKE_CURRENT_BINARY_DIR}/${META_PROJECT_NAME}")
endif()

target_link_libraries(${META_PROJECT_NAME}_tests ${TEST_LIBRARIES})
set_target_properties(${META_PROJECT_NAME}_tests PROPERTIES CXX_STANDARD 11)
add_test(NAME ${META_PROJECT_NAME}_cppunit COMMAND ${META_PROJECT_NAME}_tests -p "${CMAKE_CURRENT_SOURCE_DIR}/testfiles" ${APPLICATION_PATH})

# add the test executable to the dependencies of the check target
add_dependencies(check ${META_PROJECT_NAME}_tests)

# add target for launching tests with wine ensuring the WINEPATH is set correctly so wine is able to find all required *.dll files
# requires script from c++utilities, hence the sources of c++utilities must be present
if(MINGW AND CMAKE_CROSSCOMPILING AND CPP_UTILITIES_SOURCE_DIR)
    if(NOT TARGET ${META_PROJECT_NAME}_run_tests)
        if(CMAKE_FIND_ROOT_PATH)
            list(APPEND RUNTIME_LIBRARY_PATH "${CMAKE_FIND_ROOT_PATH}/bin")
        endif()
        add_custom_target(${META_PROJECT_NAME}_run_tests COMMAND "${CPP_UTILITIES_SOURCE_DIR}/scripts/wine.sh" "${CMAKE_CURRENT_BINARY_DIR}/${META_PROJECT_NAME}_tests.${WINDOWS_EXT}" ${RUNTIME_LIBRARY_PATH})
        add_dependencies(${META_PROJECT_NAME}_run_tests ${META_PROJECT_NAME})
    endif()
endif()
