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

// Setter pour le socket et le serveur associé
void Client::setSocketClient(int socket, Server *server){
    _ClientSocket = socket;
    _server = server;
}

// Getter pour le socket client
int Client::getSocketClient(){
    return _ClientSocket;
}

// Renvoie le buffer complet de la requête
std::string &Client::getRequest(){
    return _rawRequestBuffer;
}

// Ajoute les données reçues dans le buffer
void Client::appendRawData(const char* data, size_t len){
    _rawRequestBuffer.append(data, len);
}

// Vide le buffer une fois la requête traitée
void Client::clearRawData(){
    _rawRequestBuffer.clear();
}

// Vérifie si la requête est complète :
// 1. Les headers doivent se terminer par "\r\n\r\n".
// 2. Si un header "Content-Length:" est présent, on doit avoir reçu header + 4 + Content-Length octets.
bool Client::requestIsComplete() const {
    size_t headerEnd = _rawRequestBuffer.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
        return false; // Les headers ne sont pas encore complets

    // Extraire les headers
    std::string headers = _rawRequestBuffer.substr(0, headerEnd);
    size_t contentLength = 0;
    size_t clPos = headers.find("Content-Length:");
    if (clPos != std::string::npos) {
        // Chercher le premier chiffre après "Content-Length:"
        size_t start = headers.find_first_of("0123456789", clPos);
        if (start != std::string::npos) {
            contentLength = std::atoi(headers.c_str() + start);
        }
    }
    // Si Content-Length est défini, le buffer complet doit avoir au moins headerEnd + 4 + contentLength octets
    if (contentLength > 0)
        return (_rawRequestBuffer.size() >= headerEnd + 4 + contentLength);

    // Pour les requêtes sans body (ex. GET), la seule présence des headers suffit
    return true;
}

/*
// Méthode legacy : ajoute les données du buffer reçu (attention : pour du binaire, privilégiez l'appel à appendRawData avec la taille réelle)
bool Client::fillRequest(char *buffer){
    // On utilise strlen ici, ce qui convient pour des données texte mais pas pour du binaire.
    // Il serait préférable de passer la longueur réelle reçue depuis recv().
    appendRawData(buffer, std::strlen(buffer));
    return requestIsComplete();
}

// Version legacy de isRequestFull()
bool Client::isRequestFull(){
    return requestIsComplete();
}

// Pour les requêtes chunked, vérifie si la séquence de fin "0\r\n\r\n" est présente
bool Client::checkEnd(){
    const std::string endSequence = "0\r\n\r\n";
    if (_rawRequestBuffer.size() >= endSequence.size() &&
        _rawRequestBuffer.compare(_rawRequestBuffer.size() - endSequence.size(), endSequence.size(), endSequence) == 0)
        return true;
    return false;
}
*/
