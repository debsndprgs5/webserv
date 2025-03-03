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
    Client(int socket);
    void setSocketClient(int socket);
    int getSocketClient();
    int fillRequest(char *buffer);
   // std::string &getRequest();
};