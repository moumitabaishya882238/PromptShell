cmake_minimum_required(VERSION 3.10)
project(PromptShell LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h")

# Create executable
add_executable(promptshell ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(promptshell PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Output directory
set_target_properties(promptshell PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Compiler flags for Windows/MSYS2
if(MINGW)
    target_compile_options(promptshell PRIVATE -Wall -Wextra -Wpedantic)
endif()
