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
#include <fcntl.h>
#include "Tools.hpp"
#include "ParsingDataStructs.hpp"

#define MAX_CLIENTS 10

class Server{
	
	public :
		Server();
		~Server();
		Server(const Server &cpy);
		Server(ServerConfig Conf);
		Server &operator=(const Server &Conf);
		void setConfig(ServerConfig Conf);
		int startServer();
		void endServer();
		void printServ();
		std::string const &getName()const;
		std::string const &getIp()const;
		std::string const &getRoot()const;
		std::string const &getphpCgi()const;
		bool getSendfile()const;
		int getBodySize()const;
		unsigned int getSocketLen()const;
		LocationConfig setLocations(LocationConfig location);
		void setupLocations(std::vector<LocationConfig> config);
		std::vector<LocationConfig> _locations;//Pubilc for easy acces
		std::vector<std::string> _methods;
		std::map<std::string, std::string> _error_page;
		std::vector<int> _ports;
		std::vector<int> _sockets;
		std::vector<std::string> _access_log;
	private : 
		std::string _name;
		std::string _ipAdrs;
		std::string _root;
		int			_client_max_body_size;
		std::string _php_cgi_path;
		bool		_sendfile;
		struct sockaddr_in _socketAddress;
		unsigned int 	_socketLen;


};