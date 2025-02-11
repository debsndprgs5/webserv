#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <cctype>

using namespace std;

// ---------------------------------------------------------
// Fonctions utilitaires
// ---------------------------------------------------------

// Supprime les espaces en début et fin d'une chaîne.
string trim(const string &s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// Découpe une chaîne en tokens en se basant sur les espaces.
vector<string> split(const string &s) {
    istringstream iss(s);
    vector<string> tokens;
    string token;
    while (iss >> token)
        tokens.push_back(token);
    return tokens;
}

// Convertit une chaîne en booléen ("true", "on", "1" -> true).
bool parseBool(const string &s) {
    return (s == "true" || s == "on" || s == "1");
}

// Extrait le port depuis une chaîne du type "0.0.0.0:8000" ou "8000".
int parsePort(const string &s) {
    size_t pos = s.find(':');
    if (pos != string::npos) {
        string portStr = s.substr(pos+1);
        try {
            return stoi(portStr);
        } catch (...) {
            cerr << "Warning: Conversion du port échouée pour \"" << portStr << "\". Valeur par défaut 0 utilisée." << endl;
            return 0;
        }
    }
    try {
        return stoi(s);
    } catch (...) {
        cerr << "Warning: Conversion du port échouée pour \"" << s << "\". Valeur par défaut 0 utilisée." << endl;
        return 0;
    }
}

// ---------------------------------------------------------
// Structures de configuration
// ---------------------------------------------------------

struct LocationConfig {
    string _location_match;         // ex: "/put_test" ou "*.bla"
    string _path;                   // chemin physique ou d'alias
    string _name;                   // (optionnel)
    string _root;
    bool   _auto_index = false;
    vector<string> _methods;
    vector<string> _index;
    bool   _sendfile = false;
    string _download_dir;
    string _php_cgi_path;
};

struct ServerConfig {
    uint32_t                 _host = 0;  // adresse IP (ici non convertie en détail)
    vector<int>              _listen;
    vector<string>           _server_name;
    vector<string>           _access_log;
    vector<LocationConfig>   _location;
    map<string, string>      _error_page;
    vector<string>           _index;
    string                   _root;
    int                      _client_max_body_size = 0;
    string                   _download_dir;
    bool                     _sendfile = false;
    string                   _php_cgi_path;
};

struct HttpConfig {
    vector<string> _index;
    bool           _sendfile = false;
    int            _client_max_body_size = 0;
    string         _php_cgi_path;
};

// ---------------------------------------------------------
// Parsing fts
// ---------------------------------------------------------

// Parse a "Location" block based on a map for directives
// The opening line (ex: "location /chemin {") is passed through firstline.
LocationConfig parseLocation(ifstream &in, const string &firstLine)
{
    LocationConfig loc;
    vector<string> tokens = split(firstLine);
    if (tokens.size() < 2)
    {
        cerr << "Warning: Location block wrongly formatted, using '/' as default." << endl;
        loc._location_match = "/";
    }
    else
    {
        loc._location_match = tokens[1];
    }
    // Check if opening bracket is on the same line
    if (firstLine.find("{") == string::npos)
    {
        string line;
        if (!getline(in, line))
        {
            cerr << "Warning: Unexpected EOF after Location block declaration, incomplete block." << endl;
            return loc;
        }
        if (trim(line) != "{")
        {
            cerr << "Warning: Expected { , found: \"" << line << "\". Location block considered empty." << endl;
            return loc;
        }
    }

    // Map for location block directives.
    unordered_map<string, function<void(const vector<string>&)>> locationDirectives;
    locationDirectives["path"] = [&](const vector<string>& parts)
    {
        if (parts.size() >= 2) loc._path = parts[1];
    };
    locationDirectives["name"] = [&](const vector<string>& parts)
    {
        if (parts.size() >= 2) loc._name = parts[1];
    };
    locationDirectives["root"] = [&](const vector<string>& parts)
    {
        if (parts.size() >= 2) loc._root = parts[1];
    };
    locationDirectives["auto_index"] = [&](const vector<string>& parts)
    {
        if (parts.size() >= 2) loc._auto_index = parseBool(parts[1]);
    };
    locationDirectives["allow_methods"] = [&](const vector<string>& parts)
    {
        for (size_t i = 1; i < parts.size(); ++i)
            loc._methods.push_back(parts[i]);
    };
    locationDirectives["index"] = [&](const vector<string>& parts)
    {
        for (size_t i = 1; i < parts.size(); ++i)
            loc._index.push_back(parts[i]);
    };
    locationDirectives["sendfile"] = [&](const vector<string>& parts)
    {
        if (parts.size() >= 2) loc._sendfile = parseBool(parts[1]);
    };
    locationDirectives["download_dir"] = [&](const vector<string>& parts)
    {
        if (parts.size() >= 2) loc._download_dir = parts[1];
    };
    locationDirectives["php_cgi_path"] = [&](const vector<string>& parts)
    {
        if (parts.size() >= 2) loc._php_cgi_path = parts[1];
    };

    // Reading Location block until closing bracket is found.
    string line;
    bool closingFound = false;
    while (getline(in, line))
    {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        if (line == "}")
        {
            closingFound = true;
            break;
        }
        vector<string> parts = split(line);
        if (parts.empty()) continue;
        auto it = locationDirectives.find(parts[0]);
        if (it != locationDirectives.end())
            it->second(parts);
        else {
            cerr << "Warning: Unknown directive in Location block : " << parts[0] << endl;
        }
    }
    if (!closingFound) {
        cerr << "Warning: Closing bracket for Location block not found. Forced it to close." << endl;
    }
    return loc;
}


// Same function than previous one but for server blocks
ServerConfig parseServer(ifstream &in)
{
    ServerConfig server;

    unordered_map<string, function<void(const vector<string>&)>> serverDirectives;
    serverDirectives["host"] = [&](const vector<string>& parts)
    {
        if (parts.size() >= 2)
            server._host = static_cast<uint32_t>(stoul(parts[1]));
    };
    serverDirectives["listen"] = [&](const vector<string>& parts)
    {
        if (parts.size() >= 2)
            server._listen.push_back(parsePort(parts[1]));
    };
    serverDirectives["server_name"] = [&](const vector<string>& parts)
    {
        for (size_t i = 1; i < parts.size(); ++i)
            server._server_name.push_back(parts[i]);
    };
    serverDirectives["access_log"] = [&](const vector<string>& parts)
    {
        for (size_t i = 1; i < parts.size(); ++i)
            server._access_log.push_back(parts[i]);
    };
    serverDirectives["error_page"] = [&](const vector<string>& parts)
    {
        if (parts.size() >= 3)
            server._error_page[parts[1]] = parts[2];
    };
    serverDirectives["index"] = [&](const vector<string>& parts)
    {
        for (size_t i = 1; i < parts.size(); ++i)
            server._index.push_back(parts[i]);
    };
    serverDirectives["root"] = [&](const vector<string>& parts)
    {
        if (parts.size() >= 2)
            server._root = parts[1];
    };
    serverDirectives["client_max_body_size"] = [&](const vector<string>& parts)
    {
        if (parts.size() >= 2)
            server._client_max_body_size = stoi(parts[1]);
    };
    serverDirectives["download_dir"] = [&](const vector<string>& parts)
    {
        if (parts.size() >= 2)
            server._download_dir = parts[1];
    };
    serverDirectives["sendfile"] = [&](const vector<string>& parts)
    {
        if (parts.size() >= 2)
            server._sendfile = parseBool(parts[1]);
    };
    serverDirectives["php_cgi_path"] = [&](const vector<string>& parts)
    {
        if (parts.size() >= 2)
            server._php_cgi_path = parts[1];
    };

    string line;
    bool closingFound = false;
    while (getline(in, line))
    {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        if (line == "}")
        {
            closingFound = true;
            break;
        }
        // Detection of nested location block.
        if (line.find("location") == 0)
        {
            LocationConfig loc = parseLocation(in, line);
            server._location.push_back(loc);
        }
        else
        {
            vector<string> parts = split(line);
            if (parts.empty()) continue;
            auto it = serverDirectives.find(parts[0]);
            if (it != serverDirectives.end())
                it->second(parts);
            else
            {
                cerr << "Warning: Unknown directive in Server block : " << parts[0] << endl;
            }
        }
    }
    if (!closingFound)
        cerr << "Warning: Closing bracket for Server block not found. Forced it to close." << endl;
    return server;
}


// Same function but this time for HTTP blocks.
HttpConfig parseHttp(ifstream &in)
{
    HttpConfig http;

    unordered_map<string, function<void(const vector<string>&)>> httpDirectives;
    httpDirectives["index"] = [&](const vector<string>& parts)
    {
        for (size_t i = 1; i < parts.size(); ++i)
            http._index.push_back(parts[i]);
    };
    httpDirectives["sendfile"] = [&](const vector<string>& parts)
    {
        if (parts.size() >= 2)
            http._sendfile = parseBool(parts[1]);
    };
    httpDirectives["client_max_body_size"] = [&](const vector<string>& parts)
    {
        if (parts.size() >= 2)
            http._client_max_body_size = stoi(parts[1]);
    };
    httpDirectives["php_cgi_path"] = [&](const vector<string>& parts)
    {
        if (parts.size() >= 2)
            http._php_cgi_path = parts[1];
    };

    string line;
    bool closingFound = false;
    while (getline(in, line))
    {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        if (line == "}") {
            closingFound = true;
            break;
        }
        vector<string> parts = split(line);
        if (parts.empty()) continue;
        auto it = httpDirectives.find(parts[0]);
        if (it != httpDirectives.end())
            it->second(parts);
        else
        {
            cerr << "Warning: Unknown directive in http block : " << parts[0] << endl;
        }
    }
    if (!closingFound)
        cerr << "Warning: Closing bracket for HTTP block not found. Forced it to close." << endl;
    return http;
}


// ---------------------------------------------------------
// Parsing entry point (a modif)
// ---------------------------------------------------------
int main(int ac, char **av)
{
    ifstream infile(av[1]);
    if (!infile) {
        cerr << "Error, no config file found" << endl;
        return 1;
    }

    vector<ServerConfig> servers;
    HttpConfig httpConfig;
    string line;

    while (getline(infile, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        // Server block detection"
        if (line.find("server") == 0) {
            // Check for opening bracket.
            if (line.find("{") == string::npos) {
                if (getline(infile, line))
                {
                    if (trim(line) != "{")
                    {
                        cerr << "Warning: Expected opening brackets { after server, block ignored." << endl;
                        continue;
                    }
                }
                else
                {
                    cerr << "Warning: Unexpected EOF after server." << endl;
                    break;
                }
            }
            ServerConfig server = parseServer(infile);
            servers.push_back(server);
        }
        // Detect http block"
        else if (line.find("http") == 0)
        {
            if (line.find("{") == string::npos)
            {
                if (getline(infile, line))
                {
                    if (trim(line) != "{")
                    {
                        cerr << "Warning: Expected opening brackets { after http, block ignored." << endl;
                        continue;
                    }
                }
                else
                {
                    cerr << "Warning: Unexpected EOF after http." << endl;
                    break;
                }
            }
            httpConfig = parseHttp(infile);
        }
        // Other lines are ignored.
        else {
            cerr << "Warning: Unknown block or ignored surplus lines : " << line << endl;
        }
    }

    // Affichage pour vérification du parsing.
    cout << "Nombre de serveurs trouvés: " << servers.size() << "\n";
    for (size_t i = 0; i < servers.size(); ++i) {
        cout << "Server " << i+1 << ":\n";
        cout << "  server_name : ";
        for (const auto &name : servers[i]._server_name)
            cout << name << " ";
        cout << "\n";
        cout << "  listen ports : ";
        for (const auto &p : servers[i]._listen)
            cout << p << " ";
        cout << "\n";
        cout << "  root : " << servers[i]._root << "\n";
        cout << "  client_max_body_size : " << servers[i]._client_max_body_size << "\n";
        cout << "  Nombre de locations : " << servers[i]._location.size() << "\n";
        for (size_t j = 0; j < servers[i]._location.size(); ++j) {
            cout << "    Location " << j+1 << " (match: "
                 << servers[i]._location[j]._location_match << ") - root: "
                 << servers[i]._location[j]._root << "\n";
        }
        cout << "\n";
    }

    cout << "Configuration HTTP:\n";
    cout << "  index : ";
    for (const auto &s : httpConfig._index)
        cout << s << " ";
    cout << "\n";
    cout << "  client_max_body_size : " << httpConfig._client_max_body_size << "\n";

    return 0;
}
