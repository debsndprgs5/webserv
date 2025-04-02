#pragma once
#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Process.hpp"

#include <string>

class Server;

class Client {
private:
    int _ClientSocket;
    bool _recve_check;
    size_t _bytesSend;
    bool _sendTrigger;
    std::string _leftoverSend;
    std::string _rawRequestBuffer; // Buffer pour accumuler les données brutes de la requête

public:
    Server* _server; 

    Client();
    Client(int socket, Server *server);
    ~Client();

    Client &operator=(const Client &cpy);

    // Accesseurs et mutateurs
    void setSocketClient(int socket, Server *server);
    int getSocketClient();
    bool getRecveCheck();
    bool getSendTrigger();
    size_t getBytesSend();
    std::string getLeftover();
    void setBytesSend(size_t bytes);
    void setLeftover(std::string leftover);
    void setRecveCheck(bool state);
    void setSendTrigger(bool state);

    // Méthodes de gestion du buffer
    void appendRawData(const char* data, size_t len);
    // Vérifie si la requête est complète :
    // - Les headers sont terminés par "\r\n\r\n"
    // - Et si un header "Content-Length" est présent, le buffer contient au moins (headers + 4 + Content-Length) octets
    bool requestIsComplete() const;
    // Retourne la requête complète (buffer)
    std::string &getRequest();
    // Vide le buffer une fois la requête traitée
    void clearRawData();

    bool fillRequest(char *buffer);
    bool isRequestFull();
    bool checkEnd();
};

#endif
