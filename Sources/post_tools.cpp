#include "../Includes/Methods.hpp"

std::string Rtrim(const std::string &s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
        return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// Extract boundary from header Content-type
std::string extractBoundary(const std::string& contentType)
{
    std::string search = "boundary=";
    size_t pos = contentType.find(search);
    if (pos != std::string::npos) {
        std::string boundary = contentType.substr(pos + search.length());
        boundary = Rtrim(boundary);
        // Supprimer d'éventuels guillemets
        if (!boundary.empty() && boundary[0] == '"' && boundary[boundary.size() - 1] == '"') {
            boundary = boundary.substr(1, boundary.size() - 2);
        }
        return boundary;
    }
    return "";
}

// Split up the body using boundary
std::vector<std::string> splitBodyByBoundary(const std::string& body, const std::string& boundary)
{
    std::vector<std::string> parts;
    std::string delim = "--" + boundary;
    size_t start = 0;
    size_t end = body.find(delim, start);
    while (end != std::string::npos) {
        std::string part = body.substr(start, end - start);
        if (!part.empty())
            parts.push_back(part);
        start = end + delim.length();
        end = body.find(delim, start);
    }
    return parts;
}

// Verify if a part contain a file
bool isFilePart(const std::string& part)
{
    return part.find("filename=") != std::string::npos;
}

// Extract file name from the part
std::string Methods::extractFileName(const std::string& part)
{    _allowedTypes[".css"] = "text/css";
    size_t pos = part.find("filename=\"");
    if (pos == std::string::npos)
        return "";
    pos += 10; // longueur de 'filename="'
    size_t endPos = part.find("\"", pos);
    if (endPos == std::string::npos)
        return "";
    return part.substr(pos, endPos - pos);
}

// Extract file content from a part
std::string extractFileContent(const std::string& part)
{
    // Find the "\r\n\r\n" sequence that split headers from content
    size_t pos = part.find("\r\n\r\n");
    if (pos == std::string::npos)
        return "";
    pos += 4; // Skip the split sequence
    // Take of last CRLF that preceed the next boundary
    size_t endPos = part.rfind("\r\n");
    if (endPos == std::string::npos)
        return part.substr(pos);
    return part.substr(pos, endPos - pos);
}
