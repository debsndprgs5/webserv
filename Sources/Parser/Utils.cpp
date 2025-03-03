#include "../../Includes/Parser.hpp"

// ------------------------------------------------------------------
// Utility functions
// ------------------------------------------------------------------

// Trim whitespace from the beginning and end of a string.
std::string trim(const std::string &s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
        return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// Split a string into tokens based on whitespace.
std::vector<std::string> split(const std::string &s) {
    std::istringstream iss(s);
    std::vector<std::string> tokens;
    std::string token;
    while (iss >> token)
        tokens.push_back(token);
    return tokens;
}

// Convert a string to a boolean ("true", "on", "1" -> true).
bool parseBool(const std::string &s) {
    return (s == "true" || s == "on" || s == "1");
}

// Extract the port number from a string of the type "0.0.0.0:8000" or "8000".
int parsePort(const std::string &s) {
    size_t pos = s.find(':');
    if (pos != std::string::npos) {
        std::string portStr = s.substr(pos + 1);
        int port = std::atoi(portStr.c_str());
        if (port == 0)
            std::cerr << "Warning: Port conversion failed for \"" << portStr
                      << "\". Default value 0 used." << std::endl;
        return port;
    }
    int port = std::atoi(s.c_str());
    if (port == 0)
        std::cerr << "Warning: Port conversion failed for \"" << s
                  << "\". Default value 0 used." << std::endl;
    return port;
}
