cmake_minimum_required(VERSION 3.17.0 FATAL_ERROR)

if (NOT BASIC_PROJECT_CONFIG_DONE)
    message(FATAL_ERROR "Before including the Sphinx module, the BasicConfig module must be included.")
endif ()

option(NO_SPHINX "whether creation of Sphinx targets is disabled (enabled by default)" OFF)
if (NO_SPHINX)
    return()
endif ()

# find doxygen.h template
include(TemplateFinder)
find_template_file("sphinx-conf.py" CPP_UTILITIES SPHINX_TEMPLATE_FILE)

# find executables
find_program(SPHINX_BIN sphinx-build)
if (NOT SPHINX_BIN)
    message(WARNING "Sphinx not found, unable to add target for generating documentation for ${META_TARGET_NAME}")
    return()
endif ()

# configure paths and "conf.py"
set(SPHINX_INPUT_FILES ${SPHINX_DOC_FILES})
if (NOT SPHINX_SOURCE_DIR)
    set(SPHINX_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
endif ()
set("${META_PROJECT_VARNAME_UPPER}_SPHINX_CONFIG_DIR"
    "${CMAKE_CURRENT_BINARY_DIR}/sphinxconfig"
    CACHE PATH "config directory for Sphinx documentation")
set("${META_PROJECT_VARNAME_UPPER}_SPHINX_OUTPUT_DIR"
    "${CMAKE_CURRENT_BINARY_DIR}/doc"
    CACHE PATH "output directory for Sphinx documentation")
set(SPHINX_CONFIG_DIR "${${META_PROJECT_VARNAME_UPPER}_SPHINX_CONFIG_DIR}")
set(SPHINX_OUTPUT_DIR "${${META_PROJECT_VARNAME_UPPER}_SPHINX_OUTPUT_DIR}")
configure_file("${SPHINX_TEMPLATE_FILE}" "${SPHINX_CONFIG_DIR}/conf.py")

# add target for generating documentation
add_custom_target("${META_TARGET_NAME}_sphinxdoc" COMMAND "${SPHINX_BIN}" -c "${SPHINX_CONFIG_DIR}" "${SPHINX_SOURCE_DIR}"
                                                          "${SPHINX_OUTPUT_DIR}")
if (NOT TARGET sphinxdoc)
    add_custom_target(sphinxdoc)
endif ()
add_dependencies(sphinxdoc "${META_TARGET_NAME}_sphinxdoc")

# add install target for API documentation
if (NOT META_NO_INSTALL_TARGETS AND ENABLE_INSTALL_TARGETS)
    install(
        DIRECTORY "${SPHINX_OUTPUT_DIR}"
        DESTINATION "${META_DATA_DIR}"
        COMPONENT doc
        OPTIONAL)
    if (NOT TARGET install-doc)
        add_custom_target(install-doc COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=doc -P
                                              "${CMAKE_BINARY_DIR}/cmake_install.cmake")
    endif ()
endif ()

message(STATUS "Added target for building documentation for ${META_TARGET_NAME} with Sphinx")
