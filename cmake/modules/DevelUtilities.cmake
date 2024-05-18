cmake_minimum_required(VERSION 3.17.0 FATAL_ERROR)

# prevent multiple inclusion
if (DEFINED DEVEL_UTILITIES_LOADED)
    return()
endif ()
set(DEVEL_UTILITIES_LOADED YES)

# add option to enable defaults useful for development
option(ENABLE_DEVEL_DEFAULTS "enable build system options useful during development by default" OFF)

function (configure_development_warnings)
    # parse arguments
    set(OPTIONAL_ARGS)
    set(ONE_VALUE_ARGS TARGET OUTPUT_VAR APPEND_OUTPUT_VAR)
    set(MULTI_VALUE_ARGS)
    cmake_parse_arguments(ARGS "${OPTIONAL_ARGS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" ${ARGN})

    # enable useful warnings and explicitly disable not useful ones and treat warnings as errors
    option(ENABLE_WARNINGS "adds additional compiler flags to enable useful warnings" "${ENABLE_DEVEL_DEFAULTS}")
    set(CLANG_WARNINGS
        -Wall
        -Wextra # reasonable and standard
        -Wshadow=local # warn the user if a variable declaration shadows one from a parent context
        -Wnon-virtual-dtor # warn the user if a class with virtual functions has a non-virtual destructor. This helps catch
                           # hard to track down memory errors
        -Wold-style-cast # warn for c-style casts
        -Wcast-align # warn for potential performance problem casts
        -Wunused # warn on anything being unused
        -Woverloaded-virtual # warn if you overload (not override) a virtual function
        -Wconversion # warn on type conversions that may lose data
        -Wsign-conversion # warn on sign conversions
        -Wnull-dereference # warn if a null dereference is detected
        -Wdouble-promotion # warn if float is implicit promoted to double
        -Wformat=2 # warn on security issues around functions that format output (ie printf)
        -Wno-pedantic # warn NOT if non-standard C++ is used (some vendor extensions are very useful)
        -Wno-missing-field-initializers # warn NOT about missing field initializers
        -Wno-useless-cast # warn NOT about useless cases (this is sometimes very useful in templates)
        -Wno-unused-const-variable # warn NOT about unused constants (usually used in other compile unit)
        -Wno-unknown-warning-option # warn NOT about unknown warning options (depending on compiler/version not all are
                                    # available)
    )
    set(GCC_WARNINGS
        ${CLANG_WARNINGS}
        -Wmisleading-indentation # warn if indentation implies blocks where blocks do not exist
        -Wduplicated-cond # warn if if / else chain has duplicated conditions
        -Wduplicated-branches # warn if if / else branches have duplicated code
        -Wlogical-op # warn about logical operations being used where bitwise were probably wanted
    )
    if (ENABLE_WARNINGS)
        if (CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
            list(APPEND COMPILE_OPTIONS_TO_CONFIGURE ${CLANG_WARNINGS})
        elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            list(APPEND COMPILE_OPTIONS_TO_CONFIGURE ${GCC_WARNINGS})
        else ()
            message(AUTHOR_WARNING "Enabling warnings is not supported for compiler '${CMAKE_CXX_COMPILER_ID}'.")
        endif ()
    endif ()
    option(TREAT_WARNINGS_AS_ERRORS "adds additional compiler flag to treat warnings as errors" "${ENABLE_DEVEL_DEFAULTS}")
    if (TREAT_WARNINGS_AS_ERRORS)
        if (CMAKE_CXX_COMPILER_ID MATCHES ".*Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            list(APPEND COMPILE_OPTIONS_TO_CONFIGURE -Werror -Wno-error=address)
        else ()
            message(AUTHOR_WARNING "Treating warnings as errors is not supported for compiler '${CMAKE_CXX_COMPILER_ID}'.")
        endif ()
    endif ()

    # set compile options / set output variable
    if (ARGS_TARGET)
        target_compile_options("${ARGS_TARGET}" PRIVATE ${COMPILE_OPTIONS_TO_CONFIGURE})
    endif ()
    if (ARGS_OUTPUT_VAR)
        set("${ARGS_OUTPUT_VAR}"
            ${COMPILE_OPTIONS_TO_CONFIGURE}
            PARENT_SCOPE)
    endif ()
    if (ARGS_APPEND_OUTPUT_VAR)
        set("${ARGS_APPEND_OUTPUT_VAR}"
            ${${ARGS_APPEND_OUTPUT_VAR}} ${COMPILE_OPTIONS_TO_CONFIGURE}
            PARENT_SCOPE)
    endif ()

endfunction ()
