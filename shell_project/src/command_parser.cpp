
#include "command_parser.h"
#include "shell.h"
#include <iostream>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <cstdlib>

namespace Shell {

// CommandParser constructor
CommandParser::CommandParser() {
}

// CommandParser destructor
CommandParser::~CommandParser() {
}


// Main parsing function - parse a command line into a Command structure
std::unique_ptr<Command> CommandParser::parse_command(const std::string& line) {
    if (line.empty()) {
        return nullptr;
    }
    
    // First, parse for pipeline
    auto pipeline_parts = parse_pipeline(line);
    
    if (pipeline_parts.empty()) {
        return nullptr;
    }
    
    if (pipeline_parts.size() == 1) {
        // Single command
        return parse_single_command(pipeline_parts[0]);
    } else {
        // Pipeline detected - mark as pipeline command
        auto command = parse_single_command(pipeline_parts[0]);
        if (command) {
            command->is_pipeline = true;
        }
        return command;
    }
}


// Parse pipeline by splitting on | symbols
std::vector<std::vector<std::string>> CommandParser::parse_pipeline(const std::string& line) {
    std::vector<std::vector<std::string>> result;
    std::vector<std::string> current_tokens;
    
    std::string current_token;
    bool in_single_quote = false;
    bool in_double_quote = false;
    
    for (size_t i = 0; i < line.length(); ++i) {
        char c = line[i];
        
        // Handle escaping
        if (c == '\\' && i + 1 < line.length()) {
            current_token += line[++i];
            continue;
        }
        
        // Handle quotes
        if (c == '\'' && !in_double_quote) {
            in_single_quote = !in_single_quote;
            continue;
        }
        if (c == '"' && !in_single_quote) {
            in_double_quote = !in_double_quote;
            continue;
        }
        
        // Handle pipeline operator
        if (c == '|' && !in_single_quote && !in_double_quote) {
            if (!current_token.empty() || !current_tokens.empty()) {
                current_tokens.push_back(current_token);
                current_token.clear();
            }
            if (!current_tokens.empty()) {
                result.push_back(current_tokens);
                current_tokens.clear();
            }
            continue;
        }
        
        // Handle whitespace outside quotes
        if (std::isspace(c) && !in_single_quote && !in_double_quote) {
            if (!current_token.empty()) {
                current_tokens.push_back(current_token);
                current_token.clear();
            }
        } else {
            current_token += c;
        }
    }
    
    // Add remaining token
    if (!current_token.empty() || !current_tokens.empty()) {
        current_tokens.push_back(current_token);
        result.push_back(current_tokens);
    }
    
    return result;
}

// Parse a single command from tokens
std::unique_ptr<Command> CommandParser::parse_single_command(const std::vector<std::string>& tokens) {
    if (tokens.empty()) {
        return nullptr;
    }
    
    auto command = std::make_unique<Command>();
    std::vector<std::string> remaining_tokens = tokens;
    
    // Parse redirection and background operators
    parse_redirection(command, remaining_tokens);
    parse_background(command, remaining_tokens);
    
    // Remaining tokens are the command and its arguments
    if (!remaining_tokens.empty()) {
        command->arguments = remaining_tokens;
    }
    
    // Expand environment variables in arguments
    command->arguments = expand_variables(command->arguments);
    
    return command;
}

// Parse I/O redirection operators
void CommandParser::parse_redirection(std::unique_ptr<Command>& command, 
                                      std::vector<std::string>& tokens) {
    std::vector<std::string> new_tokens;
    
    for (size_t i = 0; i < tokens.size(); ++i) {
        const std::string& token = tokens[i];
        
        if (token == "<" && i + 1 < tokens.size()) {
            // Input redirection
            command->input_file = tokens[++i];
        } else if (token == ">" && i + 1 < tokens.size()) {
            // Output redirection (truncate)
            command->output_file = tokens[++i];
            command->append_output = false;
        } else if (token == ">>" && i + 1 < tokens.size()) {
            // Output redirection (append)
            command->output_file = tokens[++i];
            command->append_output = true;
        } else {
            new_tokens.push_back(token);
        }
    }
    
    tokens = new_tokens;
}

// Parse background execution operator
void CommandParser::parse_background(std::unique_ptr<Command>& command, 
                                     std::vector<std::string>& tokens) {
    if (!tokens.empty() && tokens.back() == "&") {
        command->background = true;
        tokens.pop_back();
    }
}

// Tokenize a command line into individual tokens
std::vector<std::string> CommandParser::tokenize(const std::string& line) {
    return tokenize_with_quotes(line);
}

// Tokenize handling quotes properly
std::vector<std::string> CommandParser::tokenize_with_quotes(const std::string& line) {
    std::vector<std::string> tokens;
    std::string current_token;
    
    bool in_single_quote = false;
    bool in_double_quote = false;
    bool escaped = false;
    
    for (size_t i = 0; i < line.length(); ++i) {
        char c = line[i];
        
        if (escaped) {
            current_token += c;
            escaped = false;
            continue;
        }
        
        if (c == '\\') {
            escaped = true;
            continue;
        }
        
        if (c == '\'' && !in_double_quote) {
            in_single_quote = !in_single_quote;
            continue;
        }
        
        if (c == '"' && !in_single_quote) {
            in_double_quote = !in_double_quote;
            continue;
        }
        
        if (std::isspace(c) && !in_single_quote && !in_double_quote) {
            if (!current_token.empty()) {
                tokens.push_back(current_token);
                current_token.clear();
            }
        } else {
            current_token += c;
        }
    }
    
    if (!current_token.empty()) {
        tokens.push_back(current_token);
    }
    
    return tokens;
}

// Process a token (expand variables, handle quotes)
std::string CommandParser::process_token(const std::string& token) {
    std::string processed = expand_variables_in_token(token);
    return processed;
}

// Expand environment variables in a single token
std::string CommandParser::expand_variables_in_token(const std::string& token) {
    std::string result;
    
    for (size_t i = 0; i < token.length(); ++i) {
        if (token[i] == '$') {
            // Look for variable name
            std::string var_name;
            size_t j = i + 1;
            
            // Handle ${variable} syntax
            if (j < token.length() && token[j] == '{') {
                j++;
                while (j < token.length() && token[j] != '}') {
                    var_name += token[j++];
                }
                if (j < token.length() && token[j] == '}') {
                    j++; // Skip closing brace
                }
                std::string var_value = get_environment_variable(var_name);
                result += var_value;
                i = j - 1; // Update position
            }
            // Handle $variable syntax
            else if (j < token.length()) {
                while (j < token.length() && (std::isalnum(token[j]) || token[j] == '_')) {
                    var_name += token[j++];
                }
                std::string var_value = get_environment_variable(var_name);
                result += var_value;
                i = j - 1; // Update position
            }
        } else {
            result += token[i];
        }
    }
    
    return result;
}

// Expand environment variables in multiple tokens
std::vector<std::string> CommandParser::expand_variables(const std::vector<std::string>& tokens) {
    std::vector<std::string> expanded;
    
    for (const auto& token : tokens) {
        expanded.push_back(expand_variables_in_token(token));
    }
    
    return expanded;
}

// Get environment variable value
std::string CommandParser::get_environment_variable(const std::string& var_name) {
    const char* value = std::getenv(var_name.c_str());
    return value ? std::string(value) : "";
}

// Utility functions
bool CommandParser::is_quote(char c) {
    return c == '"' || c == '\'';
}

bool CommandParser::is_whitespace(char c) {
    return std::isspace(static_cast<unsigned char>(c));
}

bool CommandParser::is_special_char(char c) {
    return c == '|' || c == '<' || c == '>' || c == '&' || c == '\\' || c == '$';
}

} // namespace Shell
