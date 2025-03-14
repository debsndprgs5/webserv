#include "../../Includes/Parser.hpp"

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

void setLocationAlias(LocationConfig &loc, const std::vector<std::string>& parts) {
    if (parts.size() >= 3)
        loc._alias[parts[1]] = parts[2];
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

void setLocationClientBodyBufferSize(LocationConfig &loc, const std::vector<std::string>& parts) {
    if (parts.size() >= 2)
        loc._client_body_buffer_size = std::atoi(parts[1].c_str());
}

void setLocationCgiPass(LocationConfig &loc, const std::vector<std::string>& parts) {
    if (parts.size() >= 2)
        loc._cgi_pass = parts[1];
}


// ------------------------------------------------------------------
// Parsing location blocks recursively
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
