if(NOT BASIC_PROJECT_CONFIG_DONE)
    message(FATAL_ERROR "Before including the Doxygen module, the BasicConfig module must be included.")
endif()

option(NO_DOXYGEN "whether creation of Doxygen targets is disabled (enabled by default)" OFF)
if(NO_DOXYGEN)
    return()
endif()

# find doxygen.h template
include(TemplateFinder)
find_template_file("doxygen" CPP_UTILITIES DOXYGEN_TEMPLATE_FILE)

# find executables
find_program(DOXYGEN_BIN doxygen)
find_program(PERL_BIN perl)
find_program(DIA_BIN dia)
if(DIA_BIN)
    set(HAVE_DIA "YES")
else()
    set(HAVE_DIA "NO")
endif()
find_program(DOT_BIN dot)
if(DOT_BIN)
    set(HAVE_DOT "YES")
else()
    set(HAVE_DOT "NO")
endif()

if(NOT DOXYGEN_BIN)
    message(WARNING "Doxygen not found, unable to add target for generating API documentation for ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}")
    return()
endif()

# load cached configuration and other variables
set(DOXY_LANGUAGE "English" CACHE STRING "specifies the language of the API documentation generated with Doxygen")
set(DOXY_CUSTOM_CONFIG "" CACHE STRING "specifies extra options for Doxygen")
set(DOXY_NUMBER "${META_APP_VERSION}")
set(DOXY_INPUT_FILES
    ${HEADER_FILES} ${SRC_FILES} ${TEST_HEADER_FILES} ${TEST_SRC_FILES} ${WIDGETS_HEADER_FILES} ${WIDGETS_SRC_FILES}
    ${QML_HEADER_FILES} ${QML_SRC_FILES} ${DOC_FILES} ${DOC_ONLY_FILES}
)
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
add_custom_target("${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_apidoc"
    COMMAND "${DOXYGEN_BIN}" "${CMAKE_CURRENT_BINARY_DIR}/doxygen.config"
)
if(NOT TARGET apidoc)
    add_custom_target(apidoc)
endif()
add_dependencies(apidoc "${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX}_apidoc")

# add install target for API documentation
if(NOT META_NO_INSTALL_TARGETS AND ENABLE_INSTALL_TARGETS)
    install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/api-doc"
            DESTINATION "share/${META_PROJECT_NAME}"
            COMPONENT api-doc
            OPTIONAL
    )
    if(NOT TARGET install-api-doc)
        add_custom_target(install-api-doc
            COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=api-doc -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
        )
    endif()
endif()

message(STATUS "Generating target for generating API documentation for ${TARGET_PREFIX}${META_PROJECT_NAME}${TARGET_SUFFIX} with Doxygen")
