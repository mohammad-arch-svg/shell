#ifndef SHELL_H
#define SHELL_H

#include <string>
#include <vector>
#include <memory>


namespace Shell {

// Forward declarations
class CommandParser;
class ProcessManager;
class BuiltinCommands;

/**
 * Command structure represents a parsed command with its arguments and metadata
 */
struct Command {
    std::vector<std::string> arguments;           // Command arguments
    std::string input_file;                       // Input redirection file
    std::string output_file;                      // Output redirection file
    bool append_output = false;                   // Append vs truncate output
    bool background = false;                      // Run in background
    bool is_pipeline = false;                     // This command is part of a pipeline
    
    /**
     * Check if command is empty
     */
    bool empty() const {
        return arguments.empty();
    }
    
    /**
     * Get the command name (first argument)
     */
    std::string name() const {
        return arguments.empty() ? "" : arguments[0];
    }
};

/**
 * Main Shell class that coordinates all components
 */
class Shell {
public:
    Shell();
    ~Shell();
    
    // Main shell loop
    void run();
    
private:
    // Shell state
    bool running_;
    std::string current_directory_;
    
    // Components
    std::unique_ptr<CommandParser> parser_;
    std::unique_ptr<ProcessManager> process_manager_;
    std::unique_ptr<BuiltinCommands> builtin_commands_;
    
    // Shell configuration
    std::string prompt_template_;
    std::string username_;
    std::string hostname_;
    
    // Private methods
    void initialize();
    void display_prompt();
    std::string generate_prompt();
    void execute_command(const std::string& command_line);
    void cleanup();
};

} // namespace Shell

#endif // SHELL_H
