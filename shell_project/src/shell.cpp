#include "shell.h"
#include "command_parser.h"
#include "process_manager.h"
#include "builtin_commands.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <limits.h>
#include <cstring>
#include <algorithm>

namespace Shell {

// Shell constructor - initialize all components
Shell::Shell() : running_(true), current_directory_("") {
    initialize();
}

// Shell destructor - cleanup resources
Shell::~Shell() {
    cleanup();
}

// Initialize shell components and environment
void Shell::initialize() {
    // Set up current directory
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        current_directory_ = cwd;
    }
    
    // Get username and hostname for prompt
    struct passwd* pw = getpwuid(getuid());
    if (pw != nullptr) {
        username_ = pw->pw_name;
    }
    
    char hostname[HOST_NAME_MAX];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        hostname_ = hostname;
    }
    
    // Set up prompt template
    prompt_template_ = "\\u@\\h:\\w\\$ ";
    
    // Initialize components
    parser_ = std::make_unique<CommandParser>();
    process_manager_ = std::make_unique<ProcessManager>();
    builtin_commands_ = std::make_unique<BuiltinCommands>();
    
    // Display welcome message
    std::cout << "Welcome to Modern Shell (C++20)\n";
    std::cout << "Type 'help' for available commands.\n\n";
}

// Main shell loop - read-eval-print
void Shell::run() {
    while (running_) {
        display_prompt();
        
        std::string command_line;
        if (!std::getline(std::cin, command_line)) {
            // Handle EOF (Ctrl+D)
            std::cout << "\n";
            break;
        }
        
        // Skip empty commands
        if (command_line.find_first_not_of(" \t\n\r") == std::string::npos) {
            continue;
        }
        
        // Execute the command
        execute_command(command_line);
        
        // Check for background processes periodically
        static int command_count = 0;
        if (++command_count >= 10) {
            process_manager_->wait_for_background_processes();
            command_count = 0;
        }
    }
}

// Display the shell prompt
void Shell::display_prompt() {
    std::string prompt = generate_prompt();
    std::cout << prompt;
    std::cout.flush();
}

// Generate the shell prompt with expanded escape sequences
std::string Shell::generate_prompt() {
    std::string prompt;
    
    for (size_t i = 0; i < prompt_template_.size(); ++i) {
        if (prompt_template_[i] == '\\' && i + 1 < prompt_template_.size()) {
            char escape = prompt_template_[i + 1];
            switch (escape) {
                case 'u': // Username
                    prompt += username_;
                    break;
                case 'h': // Hostname
                    prompt += hostname_;
                    break;
                case 'w': // Working directory (basename)
                    if (!current_directory_.empty()) {
                        size_t pos = current_directory_.find_last_of('/');
                        if (pos != std::string::npos && pos + 1 < current_directory_.length()) {
                            prompt += current_directory_.substr(pos + 1);
                        } else {
                            prompt += current_directory_;
                        }
                    }
                    break;
                case 'W': // Full working directory
                    prompt += current_directory_;
                    break;
                case '$': // $ or # based on user
                    prompt += (geteuid() == 0) ? "#" : "$";
                    break;
                case '\\': // Backslash literal
                    prompt += '\\';
                    break;
                default:
                    prompt += escape;
            }
            ++i; // Skip the escaped character
        } else {
            prompt += prompt_template_[i];
        }
    }
    
    return prompt;
}

// Execute a command line
void Shell::execute_command(const std::string& command_line) {
    try {
        // Parse the command
        auto command = parser_->parse_command(command_line);
        if (!command) {
            std::cerr << "Error: Failed to parse command\n";
            return;
        }
        
        // Handle built-in commands
        if (!command->arguments.empty() && 
            builtin_commands_->is_builtin(command->arguments[0])) {
            int status = builtin_commands_->execute_builtin(*command);
            if (status == -1) {  // exit command
                running_ = false;
            }
            return;
        }
        

        // Handle pipeline - currently we just execute the first command
        // In a full implementation, we would parse and execute the entire pipeline
        if (command->is_pipeline) {
            std::cout << "Pipeline detected but not fully implemented yet.\n";
            process_manager_->execute_command(*command);
        } else {
            // Execute single command
            process_manager_->execute_command(*command);
        }
        
        // Update current directory if we changed it via cd
        if (!command->arguments.empty() && command->arguments[0] == "cd") {
            char cwd[PATH_MAX];
            if (getcwd(cwd, sizeof(cwd)) != nullptr) {
                current_directory_ = cwd;
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error executing command: " << e.what() << "\n";
    } catch (...) {
        std::cerr << "Error: Unknown exception occurred\n";
    }
}

// Cleanup resources before shell exit
void Shell::cleanup() {
    if (process_manager_) {
        process_manager_->wait_for_background_processes();
    }
    
    std::cout << "\nGoodbye!\n";
}

} // namespace Shell

// Main entry point
int main() {
    Shell::Shell shell;
    shell.run();
    return 0;
}
