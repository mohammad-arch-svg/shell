


#ifndef BUILTIN_COMMANDS_H
#define BUILTIN_COMMANDS_H

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "shell.h"

namespace Shell {

// Command struct is defined in shell.h
// No forward declaration needed as it's already included

/**
 * BuiltinCommands handles implementation of shell built-in commands
 * These commands are executed directly by the shell, not as external programs
 */
class BuiltinCommands {
public:
    BuiltinCommands();
    ~BuiltinCommands();
    
    /**
     * Check if a command is a built-in command
     * @param command_name Name of the command
     * @return True if it's a built-in command
     */
    bool is_builtin(const std::string& command_name);
    
    /**
     * Execute a built-in command
     * @param command The command to execute
     * @return Exit status (0 for success, non-zero for error)
     */
    int execute_builtin(const Command& command);
    
    /**
     * Get help information about built-in commands
     * @return Help text string
     */
    std::string get_help();
    
private:
    // Built-in command implementations
    int cmd_cd(const std::vector<std::string>& args);
    int cmd_pwd(const std::vector<std::string>& args);
    int cmd_exit(const std::vector<std::string>& args);
    int cmd_help(const std::vector<std::string>& args);
    int cmd_export(const std::vector<std::string>& args);
    int cmd_echo(const std::vector<std::string>& args);
    int cmd_history(const std::vector<std::string>& args);
    
    // Utility functions for built-in commands
    std::string get_current_directory();
    bool change_directory(const std::string& path);
    std::string resolve_path(const std::string& path);
    bool is_valid_directory(const std::string& path);
    
    // Command registry
    struct BuiltinCommand {
        std::string name;
        std::string description;
        std::string usage;
        std::function<int(const std::vector<std::string>&)> handler;
    };
    
    std::vector<BuiltinCommand> builtin_commands_;
    void initialize_commands();
};

} // namespace Shell

#endif // BUILTIN_COMMANDS_H
