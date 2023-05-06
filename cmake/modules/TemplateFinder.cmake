cmake_minimum_required(VERSION 3.3.0 FATAL_ERROR)

# prevent multiple inclusion
if (DEFINED TEMPLATE_FINDER_LOADED)
    return()
endif ()
set(TEMPLATE_FINDER_LOADED YES)

function (find_template_file FILE_NAME_WITHOUT_EXTENSION PROJECT_VAR_NAME OUTPUT_VAR)
    find_template_file_full_name("${FILE_NAME_WITHOUT_EXTENSION}.in" "${PROJECT_VAR_NAME}" "${OUTPUT_VAR}")
    set(${OUTPUT_VAR} "${${OUTPUT_VAR}}" PARENT_SCOPE)
endfunction ()

function (find_template_file_full_name FILE_NAME PROJECT_VAR_NAME OUTPUT_VAR)
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/${FILE_NAME}")
        # check own source directory
        set(${OUTPUT_VAR}
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/${FILE_NAME}"
            PARENT_SCOPE)
        message(STATUS "Using template for ${FILE_NAME} from own (${META_PROJECT_NAME}) source directory.")
    elseif (EXISTS "${${PROJECT_VAR_NAME}_SOURCE_DIR}/cmake/templates/${FILE_NAME}")
        # check sources of project
        set(${OUTPUT_VAR}
            "${${PROJECT_VAR_NAME}_SOURCE_DIR}/cmake/templates/${FILE_NAME}"
            PARENT_SCOPE)
        message(STATUS "Using template for ${FILE_NAME} from ${PROJECT_VAR_NAME} source directory.")
    elseif (EXISTS "${${PROJECT_VAR_NAME}_CONFIG_DIRS}/templates/${FILE_NAME}")
        # check installed version of project
        set(${OUTPUT_VAR}
            "${${PROJECT_VAR_NAME}_CONFIG_DIRS}/templates/${FILE_NAME}"
            PARENT_SCOPE)
        message(STATUS "Using template for ${FILE_NAME} from ${PROJECT_VAR_NAME} installation.")
    else ()
        message(FATAL_ERROR "Template for ${FILE_NAME} file can not be located.")
    endif ()
endfunction ()
