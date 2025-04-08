#include "../../Includes/Parser.hpp"

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
// Parsing Http blocks
// ------------------------------------------------------------------

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