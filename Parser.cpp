#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cctype>
#include <cstdlib> // for atoi and strtoul

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

// ------------------------------------------------------------------
// Configuration structures
// ------------------------------------------------------------------

struct LocationConfig {
    std::string _location_match;  // e.g., "/put_test" or "*.bla"
    std::string _path;            // physical path or alias
    std::string _name;            // (optional)
    std::string _root;
    bool _auto_index;
    std::vector<std::string> _methods;
    std::vector<std::string> _index;
    bool _sendfile;
    std::string _download_dir;
    std::string _php_cgi_path;
    // New members for extra directives
    bool _alias;                   // flag set by "alias"
    int _client_body_buffer_size;  // set by "client_body_buffer_size"
    std::string _cgi_pass;         // set by "cgi_pass"
    // Nested location blocks
    std::vector<LocationConfig> _nested_locations;

    LocationConfig() : _auto_index(false), _sendfile(false), _alias(false),
                       _client_body_buffer_size(0) {}
};

struct ServerConfig {
    unsigned long _host;  // IP address as a number
    std::vector<int> _listen;
    std::vector<std::string> _server_name;
    std::vector<std::string> _access_log;
    std::vector<LocationConfig> _location;
    std::vector<std::string> _methods;
    std::map<std::string, std::string> _error_page;
    std::vector<std::string> _index;
    std::string _root;
    int _client_max_body_size;
    std::string _download_dir;
    bool _sendfile;
    std::string _php_cgi_path;

    ServerConfig() : _host(0), _client_max_body_size(0), _sendfile(false) {}
};

struct HttpConfig {
    std::vector<std::string> _index;
    bool _sendfile;
    int _client_max_body_size;
    std::string _php_cgi_path;

    HttpConfig() : _sendfile(false), _client_max_body_size(0) {}
};

// ------------------------------------------------------------------
// Functions to process directives in a "location" block
// ------------------------------------------------------------------

void setLocationPath(LocationConfig &loc, const std::vector<std::string>& parts) {
    if (parts.size() >= 2)
        loc._path = parts[1];
}

void setLocationName(LocationConfig &loc, const std::vector<std::string>& parts) {
    if (parts.size() >= 2)
        loc._name = parts[1];
}

void setLocationRoot(LocationConfig &loc, const std::vector<std::string>& parts) {
    if (parts.size() >= 2)
        loc._root = parts[1];
}

void setLocationAutoIndex(LocationConfig &loc, const std::vector<std::string>& parts) {
    if (parts.size() >= 2)
        loc._auto_index = parseBool(parts[1]);
}

void setLocationAllowMethods(LocationConfig &loc, const std::vector<std::string>& parts) {
    for (size_t i = 1; i < parts.size(); ++i)
        loc._methods.push_back(parts[i]);
}

void setLocationIndex(LocationConfig &loc, const std::vector<std::string>& parts) {
    for (size_t i = 1; i < parts.size(); ++i)
        loc._index.push_back(parts[i]);
}

void setLocationSendfile(LocationConfig &loc, const std::vector<std::string>& parts) {
    if (parts.size() >= 2)
        loc._sendfile = parseBool(parts[1]);
}

void setLocationDownloadDir(LocationConfig &loc, const std::vector<std::string>& parts) {
    if (parts.size() >= 2)
        loc._download_dir = parts[1];
}

void setLocationPhpCgiPath(LocationConfig &loc, const std::vector<std::string>& parts) {
    if (parts.size() >= 2)
        loc._php_cgi_path = parts[1];
}

// New directive: alias (flag only)
void setLocationAlias(LocationConfig &loc, const std::vector<std::string>& /*parts*/) {
    loc._alias = true;
}

// New directive: client_body_buffer_size
void setLocationClientBodyBufferSize(LocationConfig &loc, const std::vector<std::string>& parts) {
    if (parts.size() >= 2)
        loc._client_body_buffer_size = std::atoi(parts[1].c_str());
}

// New directive: cgi_pass
void setLocationCgiPass(LocationConfig &loc, const std::vector<std::string>& parts) {
    if (parts.size() >= 2)
        loc._cgi_pass = parts[1];
}

// ------------------------------------------------------------------
// Functions to process directives in a "server" block
// ------------------------------------------------------------------

void setServerHost(ServerConfig &server, const std::vector<std::string>& parts) {
    if (parts.size() >= 2)
        server._host = std::strtoul(parts[1].c_str(), NULL, 10);
}

void setServerListen(ServerConfig &server, const std::vector<std::string>& parts) {
    if (parts.size() >= 2)
        server._listen.push_back(parsePort(parts[1]));
}

void setServerName(ServerConfig &server, const std::vector<std::string>& parts) {
    for (size_t i = 1; i < parts.size(); ++i)
        server._server_name.push_back(parts[i]);
}

void setServerAccessLog(ServerConfig &server, const std::vector<std::string>& parts) {
    for (size_t i = 1; i < parts.size(); ++i)
        server._access_log.push_back(parts[i]);
}

void setServerErrorPage(ServerConfig &server, const std::vector<std::string>& parts) {
    if (parts.size() >= 3)
        server._error_page[parts[1]] = parts[2];
}

void setServerAllowMethods(ServerConfig &server, const std::vector<std::string>& parts) {
    for (size_t i = 1; i < parts.size(); ++i)
        server._methods.push_back(parts[i]);
}

void setServerIndex(ServerConfig &server, const std::vector<std::string>& parts) {
    for (size_t i = 1; i < parts.size(); ++i)
        server._index.push_back(parts[i]);
}

void setServerRoot(ServerConfig &server, const std::vector<std::string>& parts) {
    if (parts.size() >= 2)
        server._root = parts[1];
}

void setServerClientMaxBodySize(ServerConfig &server, const std::vector<std::string>& parts) {
    if (parts.size() >= 2)
        server._client_max_body_size = std::atoi(parts[1].c_str());
}

void setServerDownloadDir(ServerConfig &server, const std::vector<std::string>& parts) {
    if (parts.size() >= 2)
        server._download_dir = parts[1];
}

void setServerSendfile(ServerConfig &server, const std::vector<std::string>& parts) {
    if (parts.size() >= 2)
        server._sendfile = parseBool(parts[1]);
}

void setServerPhpCgiPath(ServerConfig &server, const std::vector<std::string>& parts) {
    if (parts.size() >= 2)
        server._php_cgi_path = parts[1];
}

// ------------------------------------------------------------------
// Functions to process directives in an "http" block
// ------------------------------------------------------------------

void setHttpIndex(HttpConfig &http, const std::vector<std::string>& parts) {
    for (size_t i = 1; i < parts.size(); ++i)
        http._index.push_back(parts[i]);
}

void setHttpSendfile(HttpConfig &http, const std::vector<std::string>& parts) {
    if (parts.size() >= 2)
        http._sendfile = parseBool(parts[1]);
}

void setHttpClientMaxBodySize(HttpConfig &http, const std::vector<std::string>& parts) {
    if (parts.size() >= 2)
        http._client_max_body_size = std::atoi(parts[1].c_str());
}

void setHttpPhpCgiPath(HttpConfig &http, const std::vector<std::string>& parts) {
    if (parts.size() >= 2)
        http._php_cgi_path = parts[1];
}

// ------------------------------------------------------------------
// Parsing configuration blocks
// ------------------------------------------------------------------

// Parse a "location" block based on the opening line (e.g., "location /path {")
LocationConfig parseLocation(std::ifstream &in, const std::string &firstLine)
{
    LocationConfig loc;
    std::vector<std::string> tokens = split(firstLine);
    if (tokens.size() < 2)
    {
        std::cerr << "Warning: Location block wrongly formatted, using '/' as default." << std::endl;
        loc._location_match = "/";
    }
    else
    {
        loc._location_match = tokens[1];
    }
    // Check if the opening brace is on the same line.
    if (firstLine.find("{") == std::string::npos)
    {
        std::string line;
        if (!std::getline(in, line))
        {
            std::cerr << "Warning: Unexpected EOF after Location block declaration, incomplete block." << std::endl;
            return loc;
        }
        if (trim(line) != "{")
        {
            std::cerr << "Warning: Expected '{', found: \"" << line << "\". Location block considered empty." << std::endl;
            return loc;
        }
    }

    // Create a map of directive names to processing functions.
    std::map<std::string, void (*)(LocationConfig&, const std::vector<std::string>&)> locationDirectives;
    locationDirectives["path"] = setLocationPath;
    locationDirectives["name"] = setLocationName;
    locationDirectives["root"] = setLocationRoot;
    locationDirectives["auto_index"] = setLocationAutoIndex;
    locationDirectives["allow_methods"] = setLocationAllowMethods;
    locationDirectives["index"] = setLocationIndex;
    locationDirectives["sendfile"] = setLocationSendfile;
    locationDirectives["download_dir"] = setLocationDownloadDir;
    locationDirectives["php_cgi_path"] = setLocationPhpCgiPath;
    // New directives
    locationDirectives["alias"] = setLocationAlias;
    locationDirectives["client_body_buffer_size"] = setLocationClientBodyBufferSize;
    locationDirectives["cgi_pass"] = setLocationCgiPass;

    std::string line;
    bool closingFound = false;
    while (std::getline(in, line))
    {
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;
        if (line == "}")
        {
            closingFound = true;
            break;
        }
        // Handle nested location block
        if (line.find("location") == 0) {
            LocationConfig nestedLoc = parseLocation(in, line);
            loc._nested_locations.push_back(nestedLoc);
            continue;
        }
        std::vector<std::string> parts = split(line);
        if (parts.empty())
            continue;
        std::map<std::string, void (*)(LocationConfig&, const std::vector<std::string>&)>::iterator it;
        it = locationDirectives.find(parts[0]);
        if (it != locationDirectives.end())
            it->second(loc, parts);
        else
            std::cerr << "Warning: Unknown directive in Location block: " << parts[0] << std::endl;
    }
    if (!closingFound)
        std::cerr << "Warning: Closing brace for Location block not found. Forcing closure." << std::endl;
    return loc;
}

ServerConfig parseServer(std::ifstream &in)
{
    ServerConfig server;

    std::map<std::string, void (*)(ServerConfig&, const std::vector<std::string>&)> serverDirectives;
    serverDirectives["host"] = setServerHost;
    serverDirectives["listen"] = setServerListen;
    serverDirectives["server_name"] = setServerName;
    serverDirectives["access_log"] = setServerAccessLog;
    serverDirectives["error_page"] = setServerErrorPage;
    serverDirectives["allow_methods"] = setServerAllowMethods;
    serverDirectives["index"] = setServerIndex;
    serverDirectives["root"] = setServerRoot;
    serverDirectives["client_max_body_size"] = setServerClientMaxBodySize;
    serverDirectives["download_dir"] = setServerDownloadDir;
    serverDirectives["sendfile"] = setServerSendfile;
    serverDirectives["php_cgi_path"] = setServerPhpCgiPath;

    std::string line;
    bool closingFound = false;
    while (std::getline(in, line))
    {
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;
        if (line == "}")
        {
            closingFound = true;
            break;
        }
        // Detect nested location block inside server block.
        if (line.find("location") == 0)
        {
            LocationConfig loc = parseLocation(in, line);
            server._location.push_back(loc);
        }
        else
        {
            std::vector<std::string> parts = split(line);
            if (parts.empty())
                continue;
            std::map<std::string, void (*)(ServerConfig&, const std::vector<std::string>&)>::iterator it;
            it = serverDirectives.find(parts[0]);
            if (it != serverDirectives.end())
                it->second(server, parts);
            else
                std::cerr << "Warning: Unknown directive in Server block: " << parts[0] << std::endl;
        }
    }
    if (!closingFound)
        std::cerr << "Warning: Closing brace for Server block not found. Forcing closure." << std::endl;
    return server;
}

HttpConfig parseHttp(std::ifstream &in)
{
    HttpConfig http;

    std::map<std::string, void (*)(HttpConfig&, const std::vector<std::string>&)> httpDirectives;
    httpDirectives["index"] = setHttpIndex;
    httpDirectives["sendfile"] = setHttpSendfile;
    httpDirectives["client_max_body_size"] = setHttpClientMaxBodySize;
    httpDirectives["php_cgi_path"] = setHttpPhpCgiPath;

    std::string line;
    bool closingFound = false;
    while (std::getline(in, line))
    {
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;
        if (line == "}")
        {
            closingFound = true;
            break;
        }
        std::vector<std::string> parts = split(line);
        if (parts.empty())
            continue;
        std::map<std::string, void (*)(HttpConfig&, const std::vector<std::string>&)>::iterator it;
        it = httpDirectives.find(parts[0]);
        if (it != httpDirectives.end())
            it->second(http, parts);
        else
            std::cerr << "Warning: Unknown directive in http block: " << parts[0] << std::endl;
    }
    if (!closingFound)
        std::cerr << "Warning: Closing brace for HTTP block not found. Forcing closure." << std::endl;
    return http;
}

// ------------------------------------------------------------------
// Main parsing entry point
// ------------------------------------------------------------------

int main(int argc, char **argv)
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " config_file" << std::endl;
        return 1;
    }

    std::ifstream infile(argv[1]);
    if (!infile) {
        std::cerr << "Error: config file not found" << std::endl;
        return 1;
    }

    std::vector<ServerConfig> servers;
    HttpConfig httpConfig;
    std::string line;

    while (std::getline(infile, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        // Detect server block.
        if (line.find("server") == 0) {
            // Check for opening brace.
            if (line.find("{") == std::string::npos) {
                if (std::getline(infile, line)) {
                    if (trim(line) != "{") {
                        std::cerr << "Warning: Expected opening brace '{' after server, block ignored." << std::endl;
                        continue;
                    }
                } else {
                    std::cerr << "Warning: Unexpected EOF after server." << std::endl;
                    break;
                }
            }
            ServerConfig server = parseServer(infile);
            servers.push_back(server);
        }
        // Detect http block.
        else if (line.find("http") == 0) {
            if (line.find("{") == std::string::npos) {
                if (std::getline(infile, line)) {
                    if (trim(line) != "{") {
                        std::cerr << "Warning: Expected opening brace '{' after http, block ignored." << std::endl;
                        continue;
                    }
                } else {
                    std::cerr << "Warning: Unexpected EOF after http." << std::endl;
                    break;
                }
            }
            httpConfig = parseHttp(infile);
        }
        // Other lines are ignored.
        else {
            std::cerr << "Warning: Unknown block or extra line ignored: " << line << std::endl;
        }
    }

    // Output configuration for verification.
    std::cout << "Number of servers found: " << servers.size() << "\n";
    for (size_t i = 0; i < servers.size(); ++i) {
        std::cout << "Server " << i + 1 << ":\n";
        std::cout << "  server_name: ";
        for (std::vector<std::string>::iterator it = servers[i]._server_name.begin(); it != servers[i]._server_name.end(); ++it)
            std::cout << *it << " ";
        std::cout << "\n";
        std::cout << "  listen ports: ";
        for (std::vector<int>::iterator it = servers[i]._listen.begin(); it != servers[i]._listen.end(); ++it)
            std::cout << *it << " ";
        std::cout << "\n";
        std::cout << "  root: " << servers[i]._root << "\n";
        std::cout << "  client_max_body_size: " << servers[i]._client_max_body_size << "\n";
        std::cout << "  Number of locations: " << servers[i]._location.size() << "\n";
        for (size_t j = 0; j < servers[i]._location.size(); ++j) {
            std::cout << "    Location " << j + 1 << " (match: "
                      << servers[i]._location[j]._location_match << ") - root: "
                      << servers[i]._location[j]._root << "\n";
            // Optionally, print nested locations if any.
            if (!servers[i]._location[j]._nested_locations.empty()) {
                std::cout << "      Nested locations: " << servers[i]._location[j]._nested_locations.size() << "\n";
                for (size_t k = 0; k < servers[i]._location[j]._nested_locations.size(); ++k) {
                    std::cout << "        Nested Location " << k + 1 << " (match: "
                              << servers[i]._location[j]._nested_locations[k]._location_match << ") - root: "
                              << servers[i]._location[j]._nested_locations[k]._root << "\n";
                }
            }
        }
        std::cout << "\n";
    }

    std::cout << "HTTP Configuration:\n";
    std::cout << "  index: ";
    for (std::vector<std::string>::iterator it = httpConfig._index.begin(); it != httpConfig._index.end(); ++it)
        std::cout << *it << " ";
    std::cout << "\n";
    std::cout << "  client_max_body_size: " << httpConfig._client_max_body_size << "\n";

    return 0;
}
