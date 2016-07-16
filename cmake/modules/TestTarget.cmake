# before including this module, BasicConfig must be included

# add autotools-style check target
if(NOT TARGET check)
    set(CMAKE_CTEST_COMMAND ctest -V)
    add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})
    enable_testing()
endif()

# add test executable, but exclude it from the "all target"
add_executable(${META_PROJECT_NAME}_tests EXCLUDE_FROM_ALL ${TEST_HEADER_FILES} ${TEST_SRC_FILES})
target_link_libraries(${META_PROJECT_NAME}_tests ${META_PROJECT_NAME} ${TEST_LIBRARIES} cppunit)
set_target_properties(${META_PROJECT_NAME}_tests PROPERTIES CXX_STANDARD 11)
add_test(NAME ${META_PROJECT_NAME}_cppunit COMMAND ${META_PROJECT_NAME}_tests -p "${CMAKE_CURRENT_SOURCE_DIR}/testfiles")

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
