# find doxygen.h template
include(TemplateFinder)
find_template_file("doxygen" CPP_UTILITIES DOXYGEN_TEMPLATE_FILE)

# find executables
find_program(DOXYGEN_BIN doxygen)
find_program(PERL_BIN perl)
find_program(DIA_BIN dia)
find_program(DOT_BIN dot)
if(NOT DOXYGEN_BIN)
    message(WARNING "Doxygen not found, unable to add target for generating API documentation.")

else()
    # load cached configuration and other variables
    set(DOXY_LANGUAGE "English" CACHE STRING "specifies language of the API documentation generated with Doxygen")
    set(DOXY_CUSTOM_CONFIG "" CACHE STRING "specifies extra options for Doxygen")
    set(DOXY_NUMBER 1)
    set(DOXY_INPUT_FILES ${HEADER_FILES} ${SRC_FILES} ${TEST_HEADER_FILES} ${TEST_SRC_FILES} ${WIDGETS_HEADER_FILES} ${WIDGETS_SRC_FILES} ${QML_HEADER_FILES} ${QML_SRC_FILES})
    set(DOXY_PATH_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/")
    list(GET DOC_FILES 0 DOXY_MAIN_PAGE_FILE)
    set(DOXY_MAIN_PAGE_FILE "${DOXY_PATH_PREFIX}${DOXY_MAIN_PAGE_FILE}")

    # convert DOXY_INPUT_FILES to whitespace-separated list
    include(ListToString)
    list_to_string(" " "\"${DOXY_PATH_PREFIX}" "\"" "${DOXY_INPUT_FILES}" DOXY_INPUT_FILES_WHITESPACE_SEPARATED)

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
    install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/api-doc"
            DESTINATION "share/${META_PROJECT_NAME}"
            COMPONENT api-doc
    )

    if(NOT TARGET install-api-doc)
        add_custom_target(install-api-doc
            COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=api-doc -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
        )
    endif()

endif()
