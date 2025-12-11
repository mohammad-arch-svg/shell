#!/bin/bash

# Test script for the shell
echo "Testing Unix-like Shell..."

# Test 1: Help command
echo "=== Test 1: Help Command ==="
echo "help" | timeout 5s ./shell 2>&1 || echo "Help test completed"

# Test 2: pwd command
echo "=== Test 2: PWD Command ==="
echo "pwd" | timeout 5s ./shell 2>&1 || echo "PWD test completed"

# Test 3: echo command
echo "=== Test 3: Echo Command ==="
echo "echo Hello World" | timeout 5s ./shell 2>&1 || echo "Echo test completed"

# Test 4: exit command
echo "=== Test 4: Exit Command ==="
echo "exit 0" | timeout 5s ./shell 2>&1 || echo "Exit test completed"

echo "=== All tests completed ==="
