# Modern Unix-like Shell (C++20)

A comprehensive, educational Unix-like shell implementation in modern C++20 with advanced features including pipelines, I/O redirection, environment variables, and built-in commands.

## Features

### Core Functionality
- **Read-Eval-Print Loop (REPL)** with customizable prompt
- **Command parsing** with support for quoting, escaping, and tokenization
- **Environment variable expansion** (e.g., `$HOME`, `$PATH`, `${USER}`)

### Built-in Commands
- `cd` - Change directory with proper path resolution
- `pwd` - Print current working directory  
- `exit` - Exit the shell with optional status code
- `help` - Display help information for commands
- `export` - Set environment variables
- `echo` - Display text or variables
- `history` - Command history (basic implementation)

### Advanced Features
- **External program execution** with PATH lookup
- **Background processes** using `&` operator
- **Pipeline support** using `|` to connect commands
- **I/O Redirection**:
  - `<` - Input redirection
  - `>` - Output redirection (truncate)
  - `>>` - Output redirection (append)
- **PATH resolution** for finding executables
- **Error handling** with comprehensive error messages

### Command Examples

#### Basic Commands
```bash
shell> pwd
/home/user
shell> echo Hello, World!
Hello, World!
shell> export MY_VAR=hello
shell> echo $MY_VAR
hello
```

#### Directory Navigation
```bash
shell> cd /tmp
shell> pwd
/tmp
shell> cd ~  # Go to home directory
shell> pwd
/home/user
```

#### Background Processes
```bash
shell> sleep 5 &
[1] 12345
shell> jobs
[1] Running sleep 5 &
```

#### Pipelines
```bash
shell> echo "hello world" | wc -w
2
shell> ls -la | grep ".cpp"
file1.cpp
file2.cpp
```

#### I/O Redirection
```bash
shell> echo "Hello" > output.txt
shell> cat < output.txt
Hello
shell> echo "Append" >> output.txt
shell> cat output.txt
Hello
Append
```

#### Complex Examples
```bash
shell> cat file.txt | grep "pattern" | wc -l > result.txt &
[2] 12346
shell> sort < input.txt > sorted_output.txt
```

## Building and Running

### Prerequisites
- C++20 compatible compiler (GCC 10+, Clang 10+, MSVC 2019+)
- CMake 3.16 or higher
- POSIX-compliant system (Linux, macOS, WSL)

### Build Instructions
```bash
# Clone and navigate to the project directory
cd shell_project

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build the project
make -j$(nproc)

# Run the shell
./shell
```

### Alternative Build Methods

#### Using CMake directly
```bash
mkdir build && cd build
cmake .. && make && ./shell
```

#### Using a specific compiler
```bash
cmake -DCMAKE_CXX_COMPILER=g++-10 ..
make && ./shell
```

## Architecture

The shell is implemented with a modular architecture:

### Components
1. **Shell** (`shell.cpp`) - Main coordinator and REPL loop
2. **CommandParser** (`command_parser.cpp`) - Tokenization and parsing
3. **ProcessManager** (`process_manager.cpp`) - Process execution and pipelines
4. **BuiltinCommands** (`builtin_commands.cpp`) - Built-in command implementations

### Design Principles
- **Separation of Concerns** - Each component has a specific responsibility
- **Modern C++** - Uses C++20 features like `std::unique_ptr`, lambdas, ranges
- **RAII** - Proper resource management with destructors
- **Error Handling** - Comprehensive error checking and reporting
- **Educational Value** - Well-documented code for learning purposes

## Technical Implementation

### Command Parsing
- Lexical analysis with quote handling
- Environment variable expansion
- Support for special characters and escaping
- Pipeline and redirection operator recognition

### Process Management
- POSIX `fork()` and `execvp()` for external programs
- `pipe()` for inter-process communication in pipelines
- File descriptor management for I/O redirection
- Background process tracking and cleanup

### Security Considerations
- No dangerous operations (no `system()` calls)
- Proper input validation and sanitization
- Safe file descriptor handling
- Educational and safe implementation only

## Code Structure

```
shell_project/
├── include/
│   ├── shell.h              # Main shell interface
│   ├── command_parser.h     # Command parsing interface
│   ├── process_manager.h    # Process management interface
│   └── builtin_commands.h   # Built-in commands interface
├── src/
│   ├── shell.cpp            # Main shell implementation
│   ├── command_parser.cpp   # Parser implementation
│   ├── process_manager.cpp  # Process manager implementation
│   └── builtin_commands.cpp # Built-in commands implementation
├── CMakeLists.txt           # Build configuration
└── README.md               # This file
```

## Limitations and Extensions

### Current Limitations
- Basic history implementation (no persistent history)
- Limited signal handling
- No job control beyond background processes
- No advanced shell features (aliases, functions, etc.)

### Possible Extensions
- Persistent command history
- Advanced signal handling (Ctrl+C, Ctrl+Z)
- Job control with foreground/background switching
- Shell scripting capabilities
- Command completion
- Advanced path resolution (symlinks, etc.)
- Tab completion and command history navigation

## Educational Value

This shell implementation demonstrates:
- **Systems Programming** - Process creation, file descriptors, signals
- **Parsing Techniques** - Lexical analysis, tokenization, grammar parsing
- **Modern C++** - RAII, smart pointers, lambdas, ranges
- **Software Architecture** - Separation of concerns, modular design
- **Unix Concepts** - Pipes, redirection, environment variables

## Contributing

This is an educational project. Feel free to:
- Extend functionality
- Improve code quality
- Add new features
- Fix bugs
- Enhance documentation

## License

Educational use only. This is a learning project and should not be used in production environments.

## Author

Created as an educational demonstration of Unix shell implementation in modern C++.

---

**Note**: This shell is designed for educational purposes and learning about operating systems concepts. It should not be used as a replacement for production shells like bash, zsh, or fish.
