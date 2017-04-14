# before including this module, all relevant variables must be set
# just include this module as last one since nothing should depend on it

if(NOT TARGET_CONFIG_DONE)
    message(FATAL_ERROR "Before including the ConfigHeader module, the AppTarget/LibraryTarget module must be included.")
endif()

# find config.h template
include(TemplateFinder)
find_template_file("config.h" CPP_UTILITIES CONFIG_H_TEMPLATE_FILE)

# create list of dependency versions present at link time
include(ListToString)
foreach(LINKAGE IN ITEMS "" "STATIC_")
    unset(DEPENCENCY_VERSIONS)
    unset(${LINKAGE}DEPENCENCY_VERSIONS_ARRAY)
    foreach(DEPENDENCY IN LISTS PUBLIC_${LINKAGE}LIBRARIES PRIVATE_${LINKAGE}LIBRARIES)
        if(TARGET ${DEPENDENCY})
            string(SUBSTRING "${DEPENDENCY}" 0 5 DEPENDENCY_PREFIX)
            if("${DEPENDENCY_PREFIX}" STREQUAL "Qt5::")
                string(SUBSTRING "${DEPENDENCY}" 5 -1 DEPENDENCY_MODULE_NAME)
                set(DEPENDENCY "Qt ${DEPENDENCY_MODULE_NAME}")
                set(DEPENDENCY_VER "${Qt5${DEPENDENCY_MODULE_NAME}_VERSION_STRING}")
            else()
                get_target_property(DEPENDENCY_APP_NAME "${DEPENDENCY}" APP_NAME)
                get_target_property(DEPENDENCY_VER "${DEPENDENCY}" VERSION)
                if(DEPENDENCY_APP_NAME AND NOT "${DEPENDENCY_APP_NAME}" STREQUAL "DEPENDENCY_APP_NAME-NOTFOUND")
                    set(DEPENDENCY "${DEPENDENCY_APP_NAME}")
                endif()
            endif()
            if(DEPENDENCY_VER AND NOT "${DEPENDENCY_VER}" STREQUAL "DEPENDENCY_VER-NOTFOUND")
                list(APPEND DEPENCENCY_VERSIONS "${DEPENDENCY}: ${DEPENDENCY_VER}")
            endif()
        endif()
    endforeach()
    if(DEPENCENCY_VERSIONS)
        list_to_string("," " \\\n    \"" "\"" "${DEPENCENCY_VERSIONS}" ${LINKAGE}DEPENCENCY_VERSIONS_ARRAY)
    endif()
endforeach()

# add configuration header
configure_file(
    "${CONFIG_H_TEMPLATE_FILE}"
    "${CMAKE_CURRENT_BINARY_DIR}/resources/config.h"
)

# ensure generated include files can be included via #include "resources/config.h"
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
