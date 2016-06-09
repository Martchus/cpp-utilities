# find doxygen.h template
include(TemplateFinder)
find_template_file("doxygen" CPP_UTILITIES DOXYGEN_TEMPLATE_FILE)

# find executables
find_program(DOXYGEN_BIN doxygen)
find_program(PERL_BIN perl)
if(NOT DOXYGEN_BIN)
    message(WARNING "Doxygen not found, unable to add target for generating API documentation.")
endif()

# load cached configuration and other variables
set(DOXY_LANGUAGE "English" CACHE STRING "specifies language of the API documentation generated with Doxygen")
set(DOXY_CUSTOM_CONFIG "" CACHE STRING "specifies extra options for Doxygen")
set(META_DOXY_NUMBER 1)
set(META_DOXY_LANGUAGE "${DOC_LANGUAGE}")
set(DOXY_INPUT_FILES ${HEADER_FILES} ${SRC_FILES} ${TEST_HEADER_FILES} ${TEST_SRC_FILES} ${WIDGETS_HEADER_FILES} ${WIDGETS_SRC_FILES} ${QML_HEADER_FILES} ${QML_SRC_FILES})
set(DOXY_OUTPUT_FILES "")

# convert DOXY_INPUT_FILES to whitespace-separated list
include(ListToString)
list_to_string(" " "${CMAKE_CURRENT_SOURCE_DIR}/" "${DOXY_INPUT_FILES}" DOXY_INPUT_FILES_WHITESPACE_SEPARATED)
message("string: ${DOXY_INPUT_FILES_WHITESPACE_SEPARATED}")

# generate Doxygen configuration
configure_file(
    "${DOXYGEN_TEMPLATE_FILE}"
    "${CMAKE_CURRENT_BINARY_DIR}/doxygen.config"
)

# add target for generating API documentation
add_custom_target(${META_PROJECT_NAME}_apidoc
    COMMAND "${DOXYGEN_BIN}" "${CMAKE_CURRENT_BINARY_DIR}/doxygen.config"
    SOURCES ${DOXY_INPUT_FILES}
)

# add install target for API documentation
# TODO

foreach(API_DOC_OUTPUT_FILE ${API_DOC_OUTPUT_FILES})
    get_filename_component(API_DOC_OUTPUT_DIR ${API_DOC_OUTPUT_FILE} DIRECTORY)
    install(
        FILES ${API_DOC_OUTPUT_FILE}
        DESTINATION share/${META_PROJECT_NAME}/${API_DOC_OUTPUT_DIR}
        COMPONENT cmake-modules
    )
endforeach()

if(NOT TARGET install-api-doc)
    add_custom_target(install-api-doc
        COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=api-doc -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
    )
endif()
