#pragma once
#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Process.hpp"

#include <string>

class Server;

class Client {
private:
	int				_ClientSocket;
	bool			_recve_check;
	size_t			_bytesSend;
	bool			_sendTrigger;
	std::string		_leftoverSend;
	std::string		_rawRequestBuffer;
	int				_pipe;
	int				_fd;
	int				_ret;
	int				_pid;
	std::string		_cgiOutput;

public:
	Server* _server;
	bool 			cgiHasFinished;

	Client();
	Client(int socket, Server *server);
	~Client();

	Client &operator=(const Client &cpy);

	// Accesseurs et mutateurs
	void setSocketClient(int socket, Server *server);
	int getSocketClient();
	bool getRecveCheck();
	bool getSendTrigger();
	int getCgiPid();
	int getCgiPipe() const;
	size_t getBytesSend();
	std::string getLeftover();
	void setBytesSend(size_t bytes);
	void setLeftover(std::string leftover);
	void setRecveCheck(bool state);
	void setSendTrigger(bool state);
	void setCgiPipe(int pipe);
	void setCgiPid(int fd);
	void setRet(int ret);
	void appendCgiOutput(const char *data);
	void appendCgiOutput(const std::string &data);
	std::string getCgiOutput() const;
	void clearCgiOutput();
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
