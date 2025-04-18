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



// Structure to hold a parsed HTTP request.
struct HttpRequest {
    std::string method;
    std::string uri;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body;
	int _contentLength;
};

bool parseHttpRequest(const std::string &rawRequest, HttpRequest &request);//Pour appeller la fonction en dehors ?
std::string buildHttpResponse(const std::string& body, int statusCode, const std::string& statusMessage, const std::string& server_name, const std::string& contentType);

// ------------------------------------------------------------------
// Configuration structures
// ------------------------------------------------------------------

struct LocationConfig {
    std::string _location_match;  // e.g., "/put_test" or "*.bla"
    std::string _path;            // physical path or alias
    std::string _name;            // (optional)
    std::string _root;
    std::string _default_dir_redirect;
    bool _auto_index;
    std::vector<std::string> _methods;
    std::vector<std::string> _index;
    bool _sendfile;
    std::string _alias;
    std::string _php_cgi_path;
    // New members for extra directives
    int _client_body_buffer_size;  // set by "client_body_buffer_size"
    std::string _cgi_pass;         // set by "cgi_pass"
    // Nested location blocks
    std::vector<LocationConfig> _nested_locations;

    LocationConfig() : _auto_index(false), _sendfile(false),
                       _client_body_buffer_size(0) {}
};

struct ServerConfig {
    std::string _ipAdr;
    std::string _default_dir_redirect;
    std::vector<int> _listen;
    std::string _server_name;
    std::vector<LocationConfig> _location;
    std::vector<std::string> _methods;
    std::map<std::string, std::string> _error_page;
    std::vector<std::string> _index;
    std::string _root;
    int _client_max_body_size;
    std::string _php_cgi_path;

    ServerConfig() : _client_max_body_size(0) {}
};

struct HttpConfig {
    std::vector<std::string> _index;
    bool _sendfile;
    int _client_max_body_size;
    std::string _php_cgi_path;

    HttpConfig() : _sendfile(false), _client_max_body_size(0) {}
};
