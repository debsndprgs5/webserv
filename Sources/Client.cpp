#include "../Includes/Client.hpp"
#include "../Includes/Server.hpp"
#include "../Includes/ParsingDataStructs.hpp" // Pour parseHttpRequest, etc.
#include <unistd.h>
#include <cstdlib>
#include <cstring>

Client::Client() : _ClientSocket(-1), _rawRequestBuffer("") {}

Client::Client(int socket, Server *server)
    : _ClientSocket(socket), _server(server), _rawRequestBuffer("") {
    Log("----------------Printing server THROUGH CLIENT-------------");
    Log("SERVER NAME " + _server->getName());
    Log("IP ADRS : " + _server->getIp());
}

Client::~Client(){
    close(_ClientSocket);
}

Client &Client::operator=(const Client &cpy){
    if (this != &cpy) {
        _server = cpy._server;
        _ClientSocket = cpy._ClientSocket;
        _rawRequestBuffer = cpy._rawRequestBuffer;
    }
    return *this;
}

// Set for socket and associated server
void Client::setSocketClient(int socket, Server *server){
    _ClientSocket = socket;
    _server = server;
}

// Get the client socket
int Client::getSocketClient(){
    return _ClientSocket;
}

// Returns full request buffer
std::string &Client::getRequest(){
    return _rawRequestBuffer;
}

// Adds received data in buffer
void Client::appendRawData(const char* data, size_t len){
    _rawRequestBuffer.append(data, len);
}

// Clear the buffer after processing
void Client::clearRawData(){
    _rawRequestBuffer.clear();
}


// Check if the request is complete :
// 1. Headers should finish by "\r\n\r\n".
// 2. If a "Content-Length" is present, we should have received header + 4 + Content-length bytes
bool Client::requestIsComplete() const
{
    size_t headerEnd = _rawRequestBuffer.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
        return false; // Incomplete headers

    // Extract headers
    std::string headers = _rawRequestBuffer.substr(0, headerEnd);
    size_t contentLength = 0;
    size_t clPos = headers.find("Content-Length:");
    if (clPos != std::string::npos) {
        // Find the first number after "Content-Length:"
        size_t start = headers.find_first_of("0123456789", clPos);
        if (start != std::string::npos) {
            contentLength = std::atoi(headers.c_str() + start);
        }
    }
    // If Content-Length is defined, the complete buffer should at least have headerEnd + 4 + contentLenght bytes
    if (contentLength > 0)
        return (_rawRequestBuffer.size() >= headerEnd + 4 + contentLength);

    // For requests without body like GET, headers alone are fine.
    return true;
}
