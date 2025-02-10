#pragma once

#include "Process.hpp"

class Client{

    private : 
    int _ClientSocket;

    public :
    Client();
    ~Client();
    Client(int socket);
    void setSocketClient(int socket);
    int getSocketClient();
};