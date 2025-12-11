cmake_minimum_required(VERSION 3.16)
project(ModernShell VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set compiler flags
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic")

# Add executable
add_executable(shell
    src/shell.cpp
    src/command_parser.cpp
    src/process_manager.cpp
    src/builtin_commands.cpp
)

# Set include directories
target_include_directories(shell PRIVATE include)

# Set compiler-specific options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(shell PRIVATE -pthread)
endif()

# Enable testing (optional)
enable_testing()

# Installation rules
install(TARGETS shell DESTINATION bin)
install(FILES README.md DESTINATION share/doc/${PROJECT_NAME})
