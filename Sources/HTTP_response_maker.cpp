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

// EXEMPLE POUR YONAH

void entrypoint_http_response(void)
{
    std::string body = "<html><body><h1>Hello World!</h1></body></html>";
    std::string server_name = "WebServ/1.0";

    // Construction d'une réponse 200 OK
    std::string response = buildHttpResponse(body, 200, "OK", server_name, "text/html");

    // Affichage de la réponse (en pratique tu l'envoie via un socket je crois)
    std::cout << response;
}
