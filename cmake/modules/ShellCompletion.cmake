if(NOT BASIC_PROJECT_CONFIG_DONE)
    message(FATAL_ERROR "Before including the ShellCompletion module, the BasicConfig module must be included.")
endif()

option(SHELL_COMPLETION_ENABLED "controls whether shell completion is enabled in general" ON)
option(BASH_COMPLETION_ENABLED "controls whether shell completion for bash is enabled" ON)
if(NOT SHELL_COMPLETION_ENABLED)
    return()
endif()

# add bash completion (currently the only supported shell completion)
if(BASH_COMPLETION_ENABLED)
    # find bash-completion.sh template
    include(TemplateFinder)
    find_template_file("bash-completion.sh" CPP_UTILITIES BASH_COMPLETION_TEMPLATE_FILE)

    # generate wrapper script for bash completion
    configure_file(
        "${BASH_COMPLETION_TEMPLATE_FILE}"
        "${CMAKE_CURRENT_BINARY_DIR}/bash-completion/completions/${META_PROJECT_NAME}"
        @ONLY
    )

    # add install target bash completion
    if(NOT META_NO_INSTALL_TARGETS AND ENABLE_INSTALL_TARGETS)
        install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bash-completion/completions"
                DESTINATION "share/bash-completion"
                COMPONENT bash-completion
        )
        if(NOT TARGET install-bash-completion)
            add_custom_target(install-bash-completion
                COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=bash-completion -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
            )
        endif()
    endif()

    message(STATUS "Generating files for bash completion.")
endif()
