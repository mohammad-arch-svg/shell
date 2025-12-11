



#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include <string>
#include <vector>
#include <memory>

namespace Shell {

// Forward declaration - Command is defined in shell.h
struct Command;

/**
 * CommandParser class handles parsing shell commands
 * Supports: arguments, quoting, escaping, redirection, pipelines, background execution
 */
class CommandParser {
public:
    CommandParser();
    ~CommandParser();
    
    /**
     * Parse a command line into a Command structure
     * @param line The raw command line to parse
     * @return Parsed command or nullptr if parsing fails
     */
    std::unique_ptr<Command> parse_command(const std::string& line);
    
    /**
     * Tokenize a command line into individual tokens
     * @param line The line to tokenize
     * @return Vector of tokens
     */
    std::vector<std::string> tokenize(const std::string& line);
    
    /**
     * Expand environment variables in tokens
     * @param tokens The tokens to expand
     * @return Expanded tokens with variables resolved
     */
    std::vector<std::string> expand_variables(const std::vector<std::string>& tokens);
    
private:
    // Parsing helpers

    std::vector<std::vector<std::string>> parse_pipeline(const std::string& line);
    std::unique_ptr<Command> parse_single_command(const std::vector<std::string>& tokens);
    void parse_redirection(std::unique_ptr<Command>& command, std::vector<std::string>& tokens);
    void parse_background(std::unique_ptr<Command>& command, std::vector<std::string>& tokens);
    
    // Tokenization helpers
    std::vector<std::string> tokenize_with_quotes(const std::string& line);
    std::string process_token(const std::string& token);
    std::string expand_variables_in_token(const std::string& token);
    
    // Utility functions
    bool is_quote(char c);
    bool is_whitespace(char c);
    bool is_special_char(char c);
    std::string get_environment_variable(const std::string& var_name);
};

} // namespace Shell

#endif // COMMAND_PARSER_H
