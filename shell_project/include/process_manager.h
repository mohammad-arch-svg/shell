
#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include "shell.h"

namespace Shell {

// Command struct is defined in shell.h
// No forward declaration needed as it's already included

/**
 * ProcessManager handles execution of external commands
 * Manages forking, execution, pipelines, and background processes
 */
class ProcessManager {
public:
    ProcessManager();
    ~ProcessManager();
    
    /**
     * Execute a single command
     * @param command The command to execute
     * @return Process exit status
     */
    int execute_command(const Command& command);
    
    /**
     * Execute a pipeline of commands
     * @param pipeline Vector of commands in pipeline
     * @return Process exit status
     */
    int execute_pipeline(const std::vector<std::unique_ptr<Command>>& pipeline);
    
    /**
     * Find executable in PATH
     * @param command_name Name of the command
     * @return Full path to executable or empty string if not found
     */
    std::string find_executable(const std::string& command_name);
    
    /**
     * Wait for background processes
     */
    void wait_for_background_processes();
    
    /**
     * Check if process is still running
     * @param pid Process ID
     * @return True if process is still running
     */
    bool is_process_running(pid_t pid);
    
private:
    // Process execution
    pid_t fork_process();
    int exec_external_command(const Command& command);
    int setup_io_redirection(const Command& command);
    void restore_io();
    
    // Pipeline execution
    int execute_pipeline_internal(const std::vector<std::unique_ptr<Command>>& pipeline);
    void setup_pipeline(const std::vector<std::unique_ptr<Command>>& pipeline, 
                       std::vector<int>& pipe_fds, int& num_pipes);
    
    // Background process management
    void add_background_process(pid_t pid);
    void cleanup_finished_processes();
    
    // PATH resolution
    std::string search_path(const std::string& command_name);
    bool is_executable(const std::string& path);
    
    // Member variables
    std::vector<pid_t> background_processes_;
    int saved_stdin_;
    int saved_stdout_;
    int saved_stderr_;
};

} // namespace Shell

#endif // PROCESS_MANAGER_H
