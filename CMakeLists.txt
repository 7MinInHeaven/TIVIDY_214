cmake_minimum_required(VERSION 3.14)
project(TIVIDY_Workflow_System CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Strict warnings to prevent bugs and undefined behavior
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Werror)
endif()

# Output all binaries to build/bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Include directory for header files
include_directories(include)

# -------------------------------------------------------------
# Main Application Executable
# -------------------------------------------------------------
file(GLOB_RECURSE APP_SOURCES "src/*.cpp")
add_executable(tividy_app ${APP_SOURCES})
target_include_directories(tividy_app PRIVATE include)

# -------------------------------------------------------------
# GoogleTest Integration (FetchContent for zero-friction setup)
# -------------------------------------------------------------
enable_testing()

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Automatically discover ALL tests in the tests/ folder
file(GLOB TEST_SOURCES "tests/*.cpp")

add_executable(tividy_tests ${TEST_SOURCES})
target_include_directories(tividy_tests PRIVATE include tests)
target_link_libraries(tividy_tests PRIVATE GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(tividy_tests)
