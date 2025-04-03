#pragma once
#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Process.hpp"

#include <string>

class Server;

class Client {
private:
	int				ClientSocket;
	bool			_recve_check;
	size_t			_bytesSend;
	bool			_sendTrigger;
	std::string		_leftoverSend;
	std::string		_rawRequestBuffer;
	int				pipe;
	int				fd;

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
	void setCgiPipe(int pipe);
	void setCgiPid(int fd);

	// Méthodes de gestion du buffer
	void appendRawData(const char* data, size_t len);
	bool requestIsComplete() const;
	std::string &getRequest();
	std::string getResponse(std::string content);
	void clearRawData();

	bool fillRequest(char *buffer);
	bool isRequestFull();
	bool checkEnd();
};

#endif
