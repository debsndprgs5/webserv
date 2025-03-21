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
    std::string _response;//The full message the server wiil send
    std::string _content;//The content use for a GET request(coud be void*?)
    std::map<int, std::string> _mappedCodes;//Map of errorsCodes and their definitions
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
    void myGet();
    void myPost();
    void myDelete();
    void  fillError(std::string errorCode);
	void handleRequest();
    void setResponse();
	void setConfig();
	void setConfig(LocationConfig *config);
	std::string findLocationPath(std::string uri);
    std::string findWhat();
    std::string findType(std::string uri);
	LocationConfig *findConfig(std::string path, std::vector<LocationConfig> &locations);

};

std::string extractBoundary(const std::string& contentType);
std::vector<std::string> splitBodyByBoundary(const std::string& body, const std::string& boundary);
bool isFilePart(const std::string& part);
std::string extractFileName(const std::string& part);
std::string extractFileContent(const std::string& part);

