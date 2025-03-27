#include <iostream>
#include <sstream>
#include <string>
#include <ctime>
#include <cstdlib>

std::string getCurrentDate()
{
    std::time_t now = std::time(NULL);
    char buf[100];
    std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", std::gmtime(&now));
    return std::string(buf);
}

// Function to build an HTTP response
std::string buildHttpResponse(const std::string& body, int statusCode, const std::string& statusMessage, const std::string& server_name, const std::string& contentType)
{
    std::ostringstream response;

    // Line of status
    response << "HTTP/1.1 " << statusCode << " " << statusMessage << "\r\n";

    // Headers
    response << "Date: " << getCurrentDate() << "\r\n";
    response << "Server: " << server_name << "\r\n";
    response << "Content-Type: " << contentType << "\r\n";
    response << "Content-Length: " << body.size() << "\r\n";
    response << "Connection: close\r\n"; // or "keep-alive"
    response << "\r\n"; // Empty line to split headers and body

    response << body;

    return response.str();
}
