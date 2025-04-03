#pragma once 

#include "Server.hpp"
#include "Client.hpp"
#include "Tools.hpp"
#include "Methods.hpp"
#include "ParsingDataStructs.hpp"
#include <vector>
#include <map>
#include <poll.h>
#include <csignal>
#include <cstdio>
#include <fcntl.h>
#include <sys/wait.h>

class Server;
class Client;
class Methods;


/*Avoir un tableau de serv et chaque serv ses clients*/
class Process{
    private :
    std::vector <pollfd> _fdArray; //All FDs for poll call(Clients and Serv)
    std::vector <Server*> _servArray; //All Serv instances
    std::map<int, Server*> _MappedServ; //each fd to their respective serv
    std::map<int, Client*> _MappedClient; //each servFd to respective client

	public :
	Process();
    ~Process();
    void mainLoop();
    void acceptNewClient(struct pollfd &it, std::vector<struct pollfd> &pendingClients, Server *server);
    void handleData(struct pollfd &it, std::vector<struct pollfd> &pendingDeco);
    void freeProcess();
    int  start(std::vector<ServerConfig> servers);
    void setRunning();
    void proccessData(Client *client, int fd, std::vector<struct pollfd> &pendingDeco);
    bool isPendingDeco(struct pollfd &current, std::vector<struct pollfd> &pendingDeco);
    int sendCheck(int fd, const char* data, size_t dataLength, size_t bytesSent = 0);
    int isCgiPipe(int fd);
};
void sigHandler(int sig);