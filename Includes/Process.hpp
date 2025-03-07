#pragma once 

#include "Server.hpp"
#include "Client.hpp"
#include "Tools.hpp"
#include "Methods.hpp"
#include <vector>
#include <map>
#include <poll.h>
#include <csignal>
#include <cstdio>
class Server;
class Client;
class Methods;





//here to have a responding website for testing
const std::string RESPONSE =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 33\r\n"
    "\r\n"
    "THIS IS MINE BIATCH SEE THAT SHIT";

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
    void acceptNewClient(struct pollfd &it, std::vector<struct pollfd> &pendingClients);
    void handleData(struct pollfd &it, std::vector<struct pollfd> &pendingDeco);
    void freeProcess();
    int  start(std::vector<ServerConfig> servers);
    void setRunning();
    static Process* getInstance();
    bool isPendingDeco(struct pollfd &current, std::vector<struct pollfd> &pendingDeco);
};
void sigHandler(int sig);