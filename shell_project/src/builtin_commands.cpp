

#include "builtin_commands.h"
#include "shell.h"
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <limits.h>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <sstream>

namespace Shell {

// BuiltinCommands constructor
BuiltinCommands::BuiltinCommands() {
    initialize_commands();
}

// BuiltinCommands destructor
BuiltinCommands::~BuiltinCommands() {
}

// Check if a command is a built-in command
bool BuiltinCommands::is_builtin(const std::string& command_name) {
    for (const auto& cmd : builtin_commands_) {
        if (cmd.name == command_name) {
            return true;
        }
    }
    return false;
}

// Execute a built-in command
int BuiltinCommands::execute_builtin(const Command& command) {
    if (command.arguments.empty()) {
        return 0;
    }
    
    const std::string& cmd_name = command.arguments[0];
    std::vector<std::string> args(command.arguments.begin() + 1, command.arguments.end());
    
    for (const auto& cmd : builtin_commands_) {
        if (cmd.name == cmd_name) {
            return cmd.handler(args);
        }
    }
    
    // Should not reach here if is_builtin returned true
    return 0;
}

// Get help information about built-in commands
std::string BuiltinCommands::get_help() {
    std::stringstream help;
    help << "Available built-in commands:\n\n";
    
    for (const auto& cmd : builtin_commands_) {
        help << cmd.name << " - " << cmd.description << "\n";
        if (!cmd.usage.empty()) {
            help << "  Usage: " << cmd.usage << "\n";
        }
        help << "\n";
    }
    
    help << "Additional features:\n";
    help << "  Pipeline support (|) - Connect multiple commands\n";
    help << "  I/O redirection (<, >, >>) - Redirect input/output\n";
    help << "  Background execution (&) - Run commands in background\n";
    help << "  Environment variables ($HOME, $PATH, etc.)\n";
    help << "  Quoting ('string' and \"string\")\n";
    
    return help.str();
}


// Initialize built-in command registry
void BuiltinCommands::initialize_commands() {
    builtin_commands_.clear();
    
    builtin_commands_.push_back({
        "cd",
        "Change the current directory",
        "cd [directory]",
        [this](const std::vector<std::string>& args) -> int {
            return cmd_cd(args);
        }
    });
    
    builtin_commands_.push_back({
        "pwd",
        "Print the current working directory",
        "pwd",
        [this](const std::vector<std::string>& args) -> int {
            return cmd_pwd(args);
        }
    });
    
    builtin_commands_.push_back({
        "exit",
        "Exit the shell",
        "exit [status]",
        [this](const std::vector<std::string>& args) -> int {
            return cmd_exit(args);
        }
    });
    
    builtin_commands_.push_back({
        "help",
        "Display help information",
        "help [command]",
        [this](const std::vector<std::string>& args) -> int {
            return cmd_help(args);
        }
    });
    
    builtin_commands_.push_back({
        "export",
        "Set environment variables",
        "export VARIABLE=value",
        [this](const std::vector<std::string>& args) -> int {
            return cmd_export(args);
        }
    });
    
    builtin_commands_.push_back({
        "echo",
        "Display text or variables",
        "echo [text]",
        [this](const std::vector<std::string>& args) -> int {
            return cmd_echo(args);
        }
    });
    
    builtin_commands_.push_back({
        "history",
        "Show command history (if available)",
        "history",
        [this](const std::vector<std::string>& args) -> int {
            return cmd_history(args);
        }
    });
}

// cd command implementation
int BuiltinCommands::cmd_cd(const std::vector<std::string>& args) {
    std::string target_dir;
    
    if (args.empty()) {
        // No argument - go to home directory
        const char* home = std::getenv("HOME");
        if (home) {
            target_dir = home;
        } else {
            std::cerr << "cd: HOME environment variable not set" << std::endl;
            return 1;
        }
    } else if (args.size() == 1) {
        target_dir = args[0];
    } else {
        std::cerr << "cd: too many arguments" << std::endl;
        return 1;
    }
    
    // Resolve path (handle ~, .., etc.)
    std::string resolved_path = resolve_path(target_dir);
    
    // Change directory
    if (chdir(resolved_path.c_str()) != 0) {
        std::cerr << "cd: " << target_dir << ": No such file or directory" << std::endl;
        return 1;
    }
    
    return 0;
}

// pwd command implementation
int BuiltinCommands::cmd_pwd(const std::vector<std::string>& args) {
    if (!args.empty()) {
        std::cerr << "pwd: too many arguments" << std::endl;
        return 1;
    }
    
    std::string cwd = get_current_directory();
    if (!cwd.empty()) {
        std::cout << cwd << std::endl;
        return 0;
    }
    
    return 1;
}

// exit command implementation
int BuiltinCommands::cmd_exit(const std::vector<std::string>& args) {
    int exit_status = 0;
    
    if (args.size() == 1) {
        exit_status = std::atoi(args[0].c_str());
    } else if (args.size() > 1) {
        std::cerr << "exit: too many arguments" << std::endl;
        return 1;
    }
    
    std::cout << "Goodbye!" << std::endl;
    return -1; // Special return value to signal shell to exit
}

// help command implementation
int BuiltinCommands::cmd_help(const std::vector<std::string>& args) {
    if (args.empty()) {
        // Show general help
        std::cout << get_help();
    } else if (args.size() == 1) {
        // Show help for specific command
        const std::string& cmd_name = args[0];
        bool found = false;
        
        for (const auto& cmd : builtin_commands_) {
            if (cmd.name == cmd_name) {
                found = true;
                std::cout << cmd.name << " - " << cmd.description << "\n";
                if (!cmd.usage.empty()) {
                    std::cout << "Usage: " << cmd.usage << "\n";
                }
                break;
            }
        }
        
        if (!found) {
            std::cout << "No help available for command: " << cmd_name << std::endl;
            return 1;
        }
    } else {
        std::cerr << "help: too many arguments" << std::endl;
        return 1;
    }
    
    return 0;
}

// export command implementation
int BuiltinCommands::cmd_export(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "export: missing argument" << std::endl;
        return 1;
    }
    
    for (const auto& arg : args) {
        size_t pos = arg.find('=');
        if (pos == std::string::npos) {
            std::cerr << "export: invalid argument: " << arg << std::endl;
            continue;
        }
        
        std::string var_name = arg.substr(0, pos);
        std::string var_value = arg.substr(pos + 1);
        
        if (setenv(var_name.c_str(), var_value.c_str(), 1) != 0) {
            std::cerr << "export: failed to set " << var_name << std::endl;
            return 1;
        }
    }
    
    return 0;
}

// echo command implementation
int BuiltinCommands::cmd_echo(const std::vector<std::string>& args) {
    bool first = true;
    for (const auto& arg : args) {
        if (!first) {
            std::cout << " ";
        }
        std::cout << arg;
        first = false;
    }
    std::cout << std::endl;
    return 0;
}

// history command implementation
int BuiltinCommands::cmd_history(const std::vector<std::string>& args) {
    if (!args.empty()) {
        std::cerr << "history: too many arguments" << std::endl;
        return 1;
    }
    
    // Basic implementation - in a real shell, this would read from history file
    std::cout << "Command history not implemented in this demo shell." << std::endl;
    std::cout << "In a full implementation, this would show recently executed commands." << std::endl;
    
    return 0;
}

// Utility function to get current directory
std::string BuiltinCommands::get_current_directory() {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        return std::string(cwd);
    }
    return "";
}

// Utility function to change directory
bool BuiltinCommands::change_directory(const std::string& path) {
    return chdir(path.c_str()) == 0;
}

// Utility function to resolve path
std::string BuiltinCommands::resolve_path(const std::string& path) {
    if (path.empty()) {
        return get_current_directory();
    }
    
    // Handle home directory (~)
    if (path == "~") {
        const char* home = std::getenv("HOME");
        return home ? std::string(home) : get_current_directory();
    }
    
    // Handle ~/path
    if (path.size() > 2 && path.substr(0, 2) == "~/") {
        const char* home = std::getenv("HOME");
        if (home) {
            return std::string(home) + path.substr(1);
        }
    }
    
    // For now, return path as-is
    // In a full implementation, this would resolve relative paths, symlinks, etc.
    return path;
}

// Utility function to check if directory is valid
bool BuiltinCommands::is_valid_directory(const std::string& path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

} // namespace Shell
