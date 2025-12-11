#!/bin/bash

# Build and Demo Script for Modern Unix-like Shell
# This script demonstrates building and testing the shell

echo "=== Modern Unix-like Shell (C++20) - Build and Demo ==="
echo

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo "Error: Please run this script from the shell_project directory"
    exit 1
fi

echo "1. Creating build directory..."
mkdir -p build
cd build

echo "2. Configuring with CMake..."
cmake .. || {
    echo "CMake configuration failed. Trying direct compilation..."
    cd ..
    g++ -std=c++20 -Wall -Wextra -pthread -o shell \
        src/shell.cpp src/command_parser.cpp src/process_manager.cpp src/builtin_commands.cpp \
        -I./include || {
        echo "Compilation failed. Please check your C++20 compiler."
        exit 1
    }
    echo "Direct compilation successful!"
    echo
    echo "3. Running shell tests..."
    echo
    echo "=== Shell Demo ==="
    echo
    echo "Starting shell. Try these commands:"
    echo "  pwd"
    echo "  echo 'Hello, World!'"
    echo "  export TEST_VAR=hello"
    echo "  echo \$TEST_VAR"
    echo "  help"
    echo "  exit"
    echo
    echo "Press Ctrl+D to exit the shell"
    echo
    
    exec ./shell
}

echo "3. Building with make..."
make -j$(nproc)

if [ $? -eq 0 ]; then
    echo "4. Build successful!"
    echo
    echo "=== Shell Demo ==="
    echo
    echo "Starting shell. Try these commands:"
    echo "  pwd"
    echo "  echo 'Hello, World!'"
    echo "  export TEST_VAR=hello"
    echo "  echo \$TEST_VAR"
    echo "  help"
    echo "  exit"
    echo
    echo "Press Ctrl+D to exit the shell"
    echo
    
    exec ./shell
else
    echo "Build failed. Please check the error messages above."
    exit 1
fi
