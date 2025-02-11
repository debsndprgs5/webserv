#pragma once 

#include <string>

class Config{

    private :
    	std::string _Name;
		std::string _IpAdrss;
		int			_Ports;
    public :
        Config();
        Config(std::string name, std::string Ip, int port);
        ~Config();
        std::string getName()const;
        std::string getIp()const;
        int getPort()const;
};