#include "process_manager.h"
#include "command_parser.h"
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstring>
#include <algorithm>
#include <dirent.h>
#include <sys/stat.h>

namespace Shell {

// ProcessManager constructor
ProcessManager::ProcessManager() {
    // Save standard file descriptors
    saved_stdin_ = dup(STDIN_FILENO);
    saved_stdout_ = dup(STDOUT_FILENO);
    saved_stderr_ = dup(STDERR_FILENO);
}

// ProcessManager destructor
ProcessManager::~ProcessManager() {
    // Restore standard file descriptors
    if (saved_stdin_ >= 0) close(saved_stdin_);
    if (saved_stdout_ >= 0) close(saved_stdout_);
    if (saved_stderr_ >= 0) close(saved_stderr_);
}

// Execute a single command
int ProcessManager::execute_command(const Command& command) {
    // Check if it's a built-in command
    if (command.arguments.empty()) {
        return 0;
    }
    
    const std::string& cmd_name = command.arguments[0];
    
    // Check if it's a built-in command
    if (cmd_name == "cd" || cmd_name == "pwd" || cmd_name == "exit" || 
        cmd_name == "help" || cmd_name == "echo" || cmd_name == "export") {
        // Built-in commands should be handled by BuiltinCommands
        return 0;
    }
    
    // Find the executable
    std::string executable_path = find_executable(cmd_name);
    if (executable_path.empty()) {
        std::cerr << "Command not found: " << cmd_name << std::endl;
        return -1;
    }
    
    // Fork and execute
    pid_t pid = fork();
    if (pid < 0) {
        std::cerr << "Failed to fork process" << std::endl;
        return -1;
    }
    
    if (pid == 0) {
        // Child process
        int result = exec_external_command(command);
        if (result < 0) {
            _exit(127); // Command not found
        }
        _exit(0);
    } else {
        // Parent process
        if (command.background) {
            // Background process - don't wait
            add_background_process(pid);
            return 0;
        } else {
            // Foreground process - wait for completion
            int status;
            waitpid(pid, &status, 0);
            
            if (WIFEXITED(status)) {
                return WEXITSTATUS(status);
            } else if (WIFSIGNALED(status)) {
                return 128 + WTERMSIG(status);
            }
        }
    }
    
    return 0;
}

// Execute a pipeline of commands
int ProcessManager::execute_pipeline(const std::vector<std::unique_ptr<Command>>& pipeline) {
    if (pipeline.empty()) {
        return 0;
    }
    
    if (pipeline.size() == 1) {
        return execute_command(*pipeline[0]);
    }
    
    return execute_pipeline_internal(pipeline);
}

// Execute pipeline internally using pipes
int ProcessManager::execute_pipeline_internal(const std::vector<std::unique_ptr<Command>>& pipeline) {
    int num_pipes = pipeline.size() - 1;
    std::vector<int> pipe_fds(num_pipes * 2);
    std::vector<pid_t> child_pids;
    
    // Create pipes
    for (int i = 0; i < num_pipes; ++i) {
        if (pipe(&pipe_fds[i * 2]) < 0) {
            std::cerr << "Failed to create pipe" << std::endl;
            return -1;
        }
    }
    
    // Execute each command in the pipeline
    for (size_t i = 0; i < pipeline.size(); ++i) {
        pid_t pid = fork();
        if (pid < 0) {
            std::cerr << "Failed to fork process" << std::endl;
            return -1;
        }
        
        if (pid == 0) {
            // Child process
            
            // Set up input redirection
            if (i > 0) {
                // Read from previous pipe
                dup2(pipe_fds[(i - 1) * 2], STDIN_FILENO);
            }
            
            // Set up output redirection
            if (i < pipeline.size() - 1) {
                // Write to next pipe
                dup2(pipe_fds[i * 2 + 1], STDOUT_FILENO);
            }
            
            // Close all pipe file descriptors
            for (int j = 0; j < pipe_fds.size(); ++j) {
                close(pipe_fds[j]);
            }
            
            // Execute the command
            int result = exec_external_command(*pipeline[i]);
            if (result < 0) {
                _exit(127);
            }
            _exit(0);
        } else {
            // Parent process
            child_pids.push_back(pid);
        }
    }
    
    // Close all pipe file descriptors in parent
    for (int fd : pipe_fds) {
        close(fd);
    }
    
    // Wait for all children
    int final_status = 0;
    for (pid_t pid : child_pids) {
        int status;
        waitpid(pid, &status, 0);
        
        // Return status of the last command
        if (pid == child_pids.back()) {
            if (WIFEXITED(status)) {
                final_status = WEXITSTATUS(status);
            } else if (WIFSIGNALED(status)) {
                final_status = 128 + WTERMSIG(status);
            }
        }
    }
    
    return final_status;
}

// Find executable in PATH
std::string ProcessManager::find_executable(const std::string& command_name) {
    // If command contains '/', treat as direct path
    if (command_name.find('/') != std::string::npos) {
        if (is_executable(command_name)) {
            return command_name;
        }
        return "";
    }
    
    // Search in PATH
    return search_path(command_name);
}

// Search PATH for executable
std::string ProcessManager::search_path(const std::string& command_name) {
    const char* path_env = std::getenv("PATH");
    if (!path_env) {
        return "";
    }
    
    std::string path_str(path_env);
    std::vector<std::string> path_dirs;
    
    // Split PATH by ':'
    size_t start = 0;
    size_t end = 0;
    while ((end = path_str.find(':', start)) != std::string::npos) {
        if (end > start) {
            path_dirs.push_back(path_str.substr(start, end - start));
        }
        start = end + 1;
    }
    if (start < path_str.length()) {
        path_dirs.push_back(path_str.substr(start));
    }
    
    // Search each directory
    for (const auto& dir : path_dirs) {
        std::string full_path = dir + "/" + command_name;
        if (is_executable(full_path)) {
            return full_path;
        }
    }
    
    return "";
}

// Check if a file is executable
bool ProcessManager::is_executable(const std::string& path) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        return false;
    }
    
    // Check if file exists and is executable
    return (st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH);
}

// Execute external command using execvp
int ProcessManager::exec_external_command(const Command& command) {
    if (command.arguments.empty()) {
        return -1;
    }
    
    // Set up I/O redirection
    if (setup_io_redirection(command) < 0) {
        return -1;
    }
    
    // Prepare arguments for execvp
    std::vector<const char*> argv;
    for (const auto& arg : command.arguments) {
        argv.push_back(arg.c_str());
    }
    argv.push_back(nullptr);
    
    // Execute the command
    const std::string& cmd_name = command.arguments[0];
    std::string executable_path = find_executable(cmd_name);
    if (executable_path.empty()) {
        return -1;
    }
    
    execvp(executable_path.c_str(), const_cast<char* const*>(argv.data()));
    
    // If execvp returns, an error occurred
    return -1;
}

// Set up I/O redirection
int ProcessManager::setup_io_redirection(const Command& command) {
    // Input redirection
    if (!command.input_file.empty()) {
        int fd = open(command.input_file.c_str(), O_RDONLY);
        if (fd < 0) {
            std::cerr << "Cannot open input file: " << command.input_file << std::endl;
            return -1;
        }
        if (dup2(fd, STDIN_FILENO) < 0) {
            std::cerr << "Failed to redirect input" << std::endl;
            close(fd);
            return -1;
        }
        close(fd);
    }
    
    // Output redirection
    if (!command.output_file.empty()) {
        int flags = O_WRONLY | O_CREAT;
        if (command.append_output) {
            flags |= O_APPEND;
        } else {
            flags |= O_TRUNC;
        }
        
        int fd = open(command.output_file.c_str(), flags, 0644);
        if (fd < 0) {
            std::cerr << "Cannot open output file: " << command.output_file << std::endl;
            return -1;
        }
        if (dup2(fd, STDOUT_FILENO) < 0) {
            std::cerr << "Failed to redirect output" << std::endl;
            close(fd);
            return -1;
        }
        close(fd);
    }
    
    return 0;
}

// Wait for background processes
void ProcessManager::wait_for_background_processes() {
    cleanup_finished_processes();
}

// Check if process is still running
bool ProcessManager::is_process_running(pid_t pid) {
    int status;
    pid_t result = waitpid(pid, &status, WNOHANG);
    
    if (result == 0) {
        // Process is still running
        return true;
    } else if (result == pid) {
        // Process has finished
        return false;
    } else {
        // Error or no such process
        return false;
    }
}

// Add background process to tracking list
void ProcessManager::add_background_process(pid_t pid) {
    background_processes_.push_back(pid);
}

// Clean up finished background processes
void ProcessManager::cleanup_finished_processes() {
    std::vector<pid_t> still_running;
    
    for (pid_t pid : background_processes_) {
        int status;
        pid_t result = waitpid(pid, &status, WNOHANG);
        
        if (result == 0) {
            // Process is still running
            still_running.push_back(pid);
        } else if (result == pid) {
            // Process has finished
            if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
                std::cout << "Background process " << pid << " exited with status " 
                         << WEXITSTATUS(status) << std::endl;
            } else if (WIFSIGNALED(status)) {
                std::cout << "Background process " << pid << " terminated by signal " 
                         << WTERMSIG(status) << std::endl;
            }
        }
        // If result == -1, process doesn't exist or error occurred
    }
    
    background_processes_ = still_running;
}

} // namespace Shell
