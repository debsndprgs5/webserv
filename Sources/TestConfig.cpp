#include "../Includes/TestConfig.hpp"

Config::Config(std::string name, std::string Ip, int port){

    _Name=name;
    _IpAdrss=Ip;
    _Ports=port;
}

Config::Config(){}

Config::~Config(){}

std::string Config::getName()const{
    return _Name;
}

std::string Config::getIp()const{
    return _IpAdrss;
}

int Config::getPort()const{
    return _Ports;
}