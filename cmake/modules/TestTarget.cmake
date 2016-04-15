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
