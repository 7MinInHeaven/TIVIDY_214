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

#------------------------------------------
# MailJest API 
#------------------------------------------
find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Threads REQUIRED) # mo


# -------------------------------------------------------------
# PostgreSQL (libpqxx & libpq) Dependency (Robert added this)
# -------------------------------------------------------------
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx libpq sqlite3) 
include_directories(${PQXX_INCLUDE_DIRS})
link_directories(${PQXX_LIBRARY_DIRS})

# -------------------------------------------------------------
# Main Application Executable
# -------------------------------------------------------------
file(GLOB_RECURSE APP_SOURCES "src/*.cpp")
add_executable(tividy_app ${APP_SOURCES})
target_include_directories(tividy_app PRIVATE include ${PQXX_INCLUDE_DIRS})
target_include_directories(tividy_app SYSTEM PRIVATE third_party) # mo
target_link_libraries(tividy_app PRIVATE
    ${PQXX_LIBRARIES}
    CURL::libcurl
    nlohmann_json::nlohmann_json
    Threads::Threads # mo
)
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

# Collect non-main src files so tests can use classes defined in src/
file(GLOB_RECURSE LIB_SOURCES "src/*.cpp")
list(FILTER LIB_SOURCES EXCLUDE REGEX ".*main\\.cpp$")

add_executable(tividy_tests ${TEST_SOURCES} ${LIB_SOURCES})
target_include_directories(tividy_tests PRIVATE include tests ${PQXX_INCLUDE_DIRS})
target_include_directories(tividy_tests SYSTEM PRIVATE third_party) # mo added
target_link_libraries(tividy_tests PRIVATE GTest::gtest_main ${PQXX_LIBRARIES}
    CURL::libcurl
    nlohmann_json::nlohmann_json
    Threads::Threads # mo added
)

include(GoogleTest)
gtest_discover_tests(tividy_tests)
