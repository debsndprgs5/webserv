#pragma once 
#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <cstdio>
#include "Tools.hpp"

#include "TestConfig.hpp"

#define MAX_CLIENTS 1

class Config;
class Server{
	
	public :
		Server();
		~Server();
		Server(const Server &cpy);
		Server(Config Conf);
		Server &operator=(const Server &cpy);
		void setConfig(Config Conf);
		int startServer();
		void endServer();
		std::string const &getName()const;
		std::string const getIp()const;
		int getPorts()const;
		int getSocket()const;
		unsigned int getSocketLen()const;

	private : 
		std::string _Name;
		std::string _IpAdrss;
		int			_Ports;
		int			_Socket;
		//long		_IncomeMsg;
		struct sockaddr_in _SocketAddress;
		unsigned int 	_SocketLen;
		//std::string		_ServerMsg;


};