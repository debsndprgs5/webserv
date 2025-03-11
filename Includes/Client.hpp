#pragma once

#include "Process.hpp"

class Client{

    private : 
    int _ClientSocket;
    std::string _request;

    public :
    Server *_server;
    Client();
    ~Client();
    Client(int socket, Server *server);
    Client &operator=(const Client &cpy);
    void setSocketClient(int socket, Server *server);
    int getSocketClient();
    bool fillRequest(char *buffer);
    bool isRequestFull();
    bool checkEnd();
    std::string &getRequest();
};