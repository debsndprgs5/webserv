#pragma once 

class Method{
    private : 
    Client *_client;//also have his own reference to serv(Maybe I should put it in public ?)
    int _ret;
    std::string _response;
    std::map<int, std::string> _defaultErrors; 

    public:
    Method(Client *client, Server *server);
    ~Method();
    std::string &getResponse();
    int &getRet();
    void doMethod();
    bool IsMethodAllowed(std::vector<std::string> Allowed); //check if methods is allowed(Server and Location wide)
    std::string IsinLocation();//Returns the full path if founded
    void addToLocation(void* thing, std::string path)//add the "thing" to the location specifed
    void rmFromLocation(std::string path)//delete the path
    std::string isRetSet();//check is ret is set in errorpages, return the content of the page if not returns the message for default setting
};