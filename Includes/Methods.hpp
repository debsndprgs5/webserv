#pragma once 
#include "Process.hpp"
#include "Client.hpp"
#include "ParsingDataStructs.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <unistd.h>
#include <poll.h>
#include <sys/stat.h>

class Client;

class Methods{
    private : 
    Client *_client;//also have his own reference to serv(Maybe I should put it in public ?)
    int _ret;
	int _buffer_size;
    HttpRequest _parsedRequest;
	std::string _pathWithAlias;//Path Updated with aliases
    std::string _response;//The full message the server wiil send
    std::string _content;//The content use for a GET request(coud be void*?)
	std::string _cgiName;
	std::string _cgiPath;
	std::string _cgiArg;
    std::map<int, std::string> _mappedCodes;//Map of errorsCodes and their definitions
    std::map<std::string, std::string> _defaultErrors;//error_code and path to the page
	std::map<std::string, std::string> _allowedTypes;
	std::string _root;
	std::vector<std::string> _methods;
	std::string _php_cgi_path;
	    

    public:
	std::vector <struct pollfd> &_fdArray;
    Methods(Client *client, HttpRequest parsedRequest, std::vector<struct pollfd> &fdArray);
    ~Methods();
    std::string parsedUri(int trigger);
    std::string &getResponse();
	std::string findPath();
    void doMethod();
	bool checkPhpCgi();//returns true in founded cgi in uri
    bool isMethodAllowed(std::vector<std::string> Allowed, std::string method); //check if methods is allowed(Server and Location wide)
    void myGet();
    void myPost();
    void myDelete();
    void  fillError(std::string errorCode);
	void handleRequest();
    void setResponse();
	bool setCgiPath();
	void setCgiName();
	void setCgiArg();
	void cgiHandler();
	std::string extractFileName(const std::string& part);
	void setConfig();
	void setConfig(LocationConfig *config);
	std::string findLocationPath(std::string uri);
    std::string findWhat();
    std::string findType(std::string uri);
	LocationConfig *findConfig(std::string path, std::vector<LocationConfig> &locations);
    void cgi_php_handler(int *ret, const char *scriptname, std::string *querystring, bool reqtype, const char *path, int fdtemp, std::string uri);

};
std::string runCgiAndGetOutput(const char *scriptname, std::string &queryString, bool reqType, const char *path, int *ret, std::string uri);
std::string extractBoundary(const std::string& contentType);
std::vector<std::string> splitBodyByBoundary(const std::string& body, const std::string& boundary);
bool isFilePart(const std::string& part);
std::string extractFileName(const std::string& part);
std::string extractFileContent(const std::string& part);
