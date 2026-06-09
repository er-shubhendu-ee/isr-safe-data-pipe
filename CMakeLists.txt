cmake_minimum_required(VERSION 3.10)

project(service_pipe LANGUAGES C)

# -----------------------------------------------------------------------------
# Library
# -----------------------------------------------------------------------------

add_library(service_pipe STATIC
    service_pipe.c
)

# -----------------------------------------------------------------------------
# Build Mode
# -----------------------------------------------------------------------------

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)

    message(STATUS "Standalone build")

    set(SERVICE_PIPE_BUILD_TESTS_DEFAULT ON)

    target_compile_definitions(service_pipe
        PUBLIC
            SERVICE_PIPE_STANDALONE=1
    )

else()

    message(STATUS "Submodule build")

    set(SERVICE_PIPE_BUILD_TESTS_DEFAULT OFF)

endif()

option(SERVICE_PIPE_BUILD_TESTS
       "Build tests"
       ${SERVICE_PIPE_BUILD_TESTS_DEFAULT})

# -----------------------------------------------------------------------------
# Include Directories
# -----------------------------------------------------------------------------

target_include_directories(service_pipe
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/suppliment
)

target_compile_features(service_pipe
    PUBLIC
        c_std_99
)

# -----------------------------------------------------------------------------
# Warnings
# -----------------------------------------------------------------------------

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

# -----------------------------------------------------------------------------
# Tests
# -----------------------------------------------------------------------------

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