#pragma once 
#include "Process.hpp"

class Methods{
    private : 
    Client *_client;//also have his own reference to serv(Maybe I should put it in public ?)
    int _ret;
    struct HttpRequest _parsedRequest;
	std::string _responseBody;//Headers and shit ? (idealy _responseBody + _content = _response)
    std::string _response;//The full message the server wiil send
    std::string _content;//The content use for a GET request(coud be void*?)
    std::string _safePost;//Part after last slash in a POST request
    std::map<std::string, std::string> _defaultErrors;//error_code and path to the page
	std::map<> _allowedTypes;

    public:
    Method(Client *client);
    ~Method();
    std::string parsedUri(int trigger);
    std::string &getResponse();
    void doMethod();
    bool isMethodAllowed(std::vector<std::string> Allowed, std::string method); //check if methods is allowed(Server and Location wide)
    LocationConfig *findConfig(const std::string &requestedPath, std::vector<LocationConfig> &locations);//Returns the config if founded
    void myGet(LocationConfig Conf, std::string path);
    void myPost(LocationConfig Conf, std::string path);
    void myDelete(LocationConfig Conf, std::string path);
    void  fillError(std::string errorCode);
    void setResponse();
};
