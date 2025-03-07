#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <cstdlib>  // for atoi
#include "ParsingDataStructs.hpp"

// Utility function: trim whitespace from both ends of a string.
std::string trim(const std::string &s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
        return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}


// Function to parse a raw HTTP request string.
bool parseHttpRequest(const std::string &rawRequest, HttpRequest &request) {
    std::istringstream stream(rawRequest);
    std::string line;

    // --- Parse the request line (e.g., "GET /index.html HTTP/1.1") ---
    if (!std::getline(stream, line))
        return false;
    line = trim(line);
    if (line.empty())
        return false;
    std::istringstream requestLineStream(line);
    // Make tokens out of the request
    if (!(requestLineStream >> request.method >> request.uri >> request.version))
        return false;

    // --- Parse HTTP headers ---
    while (std::getline(stream, line)) {
        line = trim(line);
        if (line.empty())
            break; // Fin des headers (la ligne vide sépare headers et body)
        size_t pos = line.find(':');
        if (pos == std::string::npos)
            continue;  // Ligne mal formée, on passe
        std::string key = trim(line.substr(0, pos));
        std::string value = trim(line.substr(pos + 1));
        request.headers[key] = value;
    }

    // --- Parse the body if Content-Length header is provided ---
    std::map<std::string, std::string>::iterator it = request.headers.find("Content-Length");
    if (it != request.headers.end()) {
        int contentLength = std::atoi(it->second.c_str());
        if (contentLength > 0) {
            // Read exactly contentLength bytes from the stream
            std::vector<char> buffer(contentLength);
            stream.read(&buffer[0], contentLength);
            request.body.assign(&buffer[0], contentLength);
        }
    }

    return true;
}



// Example usage.

// Fonction principale prend la string de la requete brute et une struct httprequest vide initialisée

// Exemple d'une requête HTTP brute.
//    std::string rawRequest = "POST /submit-form HTTP/1.1\r\n"
//                             "Host: example.com\r\n"
//                             "User-Agent: CppServer/1.0\r\n"
//                             "Content-Length: 11\r\n"
//                             "\r\n"
//                             "Hello World";
//
//    HttpRequest req;
//    if (parseHttpRequest(rawRequest, req)) {
//        std::cout << "Méthode   : " << req.method << "\n";
//        std::cout << "URI       : " << req.uri << "\n";
//        std::cout << "Version   : " << req.version << "\n";
//        std::cout << "Headers   :\n";
//        for (std::map<std::string, std::string>::iterator it = req.headers.begin(); it != req.headers.end(); ++it) {
//            std::cout << "  " << it->first << ": " << it->second << "\n";
//        }
//        std::cout << "Body      : " << req.body << "\n";
//    } else {
//        std::cerr << "Erreur lors du parsing de la requête HTTP.\n";
//    }
//
//    return 0;
//
