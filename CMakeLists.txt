cmake_minimum_required(VERSION 3.10)

# -----------------------------------------------------------------------------
# Build Mode Detection
# -----------------------------------------------------------------------------

# ESP-IDF sets IDF_PATH when processing an ESP-IDF component.
if(DEFINED ENV{IDF_PATH} AND DEFINED ESP_PLATFORM)

    # =========================================================================
    # ESP-IDF COMPONENT
    # =========================================================================

    message(STATUS "service_pipe: ESP-IDF component build")

    idf_component_register(
        SRCS
            service_pipe.c

        INCLUDE_DIRS
            "."
            "suppliment"

        PRIV_REQUIRES
            service_base
            service_system
    )

else()

    # =========================================================================
    # STANDALONE CMAKE PROJECT
    # =========================================================================

    project(service_pipe LANGUAGES C)

    message(STATUS "service_pipe: Standalone CMake build")

    # -------------------------------------------------------------------------
    # Library
    # -------------------------------------------------------------------------

    add_library(service_pipe STATIC
        service_pipe.c
    )

    target_include_directories(service_pipe
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/suppliment
    )

    target_compile_features(service_pipe
        PUBLIC
            c_std_99
    )

    # -------------------------------------------------------------------------
    # Standalone build definition
    # -------------------------------------------------------------------------

    target_compile_definitions(service_pipe
        PUBLIC
            SERVICE_PIPE_STANDALONE=1
    )

    # -------------------------------------------------------------------------
    # Warnings
    # -------------------------------------------------------------------------

    if(MSVC)

        target_compile_options(service_pipe
            PRIVATE
                /W4
        )

    else()

        target_compile_options(service_pipe
            PRIVATE
                -Wall
                -Wextra
                -Wpedantic
                -Wconversion
                -Wshadow
        )

    endif()

    # -------------------------------------------------------------------------
    # Tests
    # -------------------------------------------------------------------------

    option(
        SERVICE_PIPE_BUILD_TESTS
        "Build service_pipe tests"
        ON
    )

    if(SERVICE_PIPE_BUILD_TESTS)

        file(GLOB TEST_SOURCES
            test/*.c
        )

        add_executable(service_pipe_test
            ${TEST_SOURCES}
        )

        target_link_libraries(service_pipe_test
            PRIVATE
                service_pipe
        )

    endif()

endif()