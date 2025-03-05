#pragma once
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
    std::string _ipAdr;
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

    ServerConfig() : _client_max_body_size(0), _sendfile(false) {}
};

struct HttpConfig {
    std::vector<std::string> _index;
    bool _sendfile;
    int _client_max_body_size;
    std::string _php_cgi_path;

    HttpConfig() : _sendfile(false), _client_max_body_size(0) {}
};
