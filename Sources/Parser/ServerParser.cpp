#include "../../Includes/Parser.hpp"

// ------------------------------------------------------------------
// Functions to process directives in a "server" block
// ------------------------------------------------------------------

void setServerListen(ServerConfig &server, const std::vector<std::string>& parts) {
    if (parts.size() >= 2)
    {
        server._listen.push_back(parsePort(parts[1]));
        server._ipAdr = parseIP(parts[1]);
    }
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
// Parsing Server blocks
// ------------------------------------------------------------------

ServerConfig parseServer(std::ifstream &in)
{
    ServerConfig server;

    std::map<std::string, void (*)(ServerConfig&, const std::vector<std::string>&)> serverDirectives;
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