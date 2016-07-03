set(SHELL_COMPLETION_ENABLED "yes" CACHE STRING "controls whether shell completion is enabled")
set(BASH_COMPLETION_ENABLED "yes" CACHE STRING "controls whether bash completion is enabled")

if(${SHELL_COMPLETION_ENABLED} STREQUAL "yes")

    if(NOT COMPLETION_META_PROJECT_NAME)
        set(COMPLETION_META_PROJECT_NAME ${META_PROJECT_NAME})
    endif()

    # add bash completion (currently the only supported shell completion)
    if(${BASH_COMPLETION_ENABLED} STREQUAL "yes")
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
        install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bash-completion/completions"
                DESTINATION "share/${META_PROJECT_NAME}/share"
                COMPONENT bash-completion
        )
        if(NOT TARGET install-bash-completion)
            add_custom_target(install-bash-completion
                COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=bash-completion -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
            )
        endif()
    endif()

endif()
