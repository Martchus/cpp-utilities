# before including this module, all relevant variables must be set
# just include this module as last one since nothing should depend on it

if(NOT TARGET_CONFIG_DONE)
    message(FATAL_ERROR "Before including the ConfigHeader module, the BasicConfig module must be included.")
endif()

# find config.h template
include(TemplateFinder)
find_template_file("config.h" CPP_UTILITIES CONFIG_H_TEMPLATE_FILE)

# add configuration header
configure_file(
    "${CONFIG_H_TEMPLATE_FILE}"
    "${CMAKE_CURRENT_BINARY_DIR}/resources/config.h"
)

# ensure generated include files can be included via #include "resources/config.h"
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
