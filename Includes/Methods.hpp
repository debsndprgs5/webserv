#pragma once 
#include "Process.hpp"
#include "Client.hpp"
#include "ParsingDataStructs.hpp"
class Client;

class Methods{
    private : 
    Client *_client;//also have his own reference to serv(Maybe I should put it in public ?)
    int _ret;
    HttpRequest _parsedRequest;
	std::string _responseBody;//Headers and shit ? (idealy _responseBody + _content = _response)
    std::string _response;//The full message the server wiil send
    std::string _content;//The content use for a GET request(coud be void*?)
    std::map<std::string, std::string> _defaultErrors;//error_code and path to the page
	std::map<std::string, std::string> _allowedTypes;
	std::string _root;
	std::vector<std::string> _methods;
	std::string _php_cgi_path;
	std::string _download_dir;

    public:
    Methods(Client *client, HttpRequest parsedRequest);
    ~Methods();
    std::string parsedUri(int trigger);
    std::string &getResponse();
    void doMethod();
    bool isMethodAllowed(std::vector<std::string> Allowed, std::string method); //check if methods is allowed(Server and Location wide)
    LocationConfig *findConfig(const std::string &requestedPath, std::vector<LocationConfig> &locations);//Returns the config if founded
    void myGet();
    void myPost();
    void myDelete();
    void  fillError(std::string errorCode);
	void handleRequest();
    void setResponse();
	void setConfig();
	void setConfig(LocationConfig config);
	std::string findLocationPath(std::string uri);
	LocationConfig findConfig(std::string path, std::vector<LocationConfig> &locations);
};
