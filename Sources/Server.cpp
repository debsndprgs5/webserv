#include "../Includes/Server.hpp"

//Should do nothing here
Server::Server(){
}

Server::~Server(){
	std::cout << _name << " ending service ...." << std::endl;
	endServer();
}

//Config all variables according to Config item
Server::Server(ServerConfig Conf){
	this->setConfig(Conf);
}

Server::Server(const Server &cpy){
	*this=cpy;
}

Server &Server::operator=(const Server &Conf){
	_name = Conf._name;
	_ipAdrs = Conf._ipAdrs;
	_ports = Conf._ports;
	_methods = Conf._methods;
	_error_page = Conf._error_page;
	_root = Conf._root;
	_access_log = Conf._access_log;
	_client_max_body_size = Conf._client_max_body_size;
	_sendfile = Conf._sendfile;
	_php_cgi_path = Conf._php_cgi_path;
	_sockets = Conf._sockets;
	_socketAddress = Conf._socketAddress;
	_socketLen = Conf._socketLen;
	setupLocations(Conf._locations);
	return *this;
}

void Server::setConfig(ServerConfig Conf){
	_name = *Conf._server_name.begin();
	_ipAdrs = Conf._ipAdr;
	_ports = Conf._listen;
	_methods = Conf._methods;
	_error_page = Conf._error_page;
	_root = Conf._root;
	_access_log = Conf._access_log;
	_client_max_body_size = Conf._client_max_body_size;
	_sendfile = Conf._sendfile;
	_php_cgi_path = Conf._php_cgi_path;
	memset(&_socketAddress, 0, sizeof(_socketAddress));
	_socketLen = sizeof(struct sockaddr_in);
	_socketAddress.sin_family = AF_INET;
	_socketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	setupLocations(Conf._location);
	if(!_client_max_body_size)
		_client_max_body_size = 2359297;
}


//Copy and returns full setup Location
LocationConfig Server::setLocations(LocationConfig location){

	LocationConfig Conf;
	Conf = location;
	if(Conf._root.empty())
		Conf._root = _root;
	if(Conf._methods.empty())
		Conf._methods = _methods;
	if(Conf._php_cgi_path.empty())
		Conf._php_cgi_path = _php_cgi_path;
	if(!Conf._nested_locations.empty()){
		size_t i = 0;
		for(std::vector<LocationConfig>::iterator it = location._nested_locations.begin(); it != location._nested_locations.end(); it ++){
			Conf._nested_locations[i] = setLocations(*it);
			i++;
		}
	}
	return (Conf);
}

void Server::setupLocations(std::vector<LocationConfig> config){
	for(std::vector<LocationConfig>::iterator it= config.begin(); it != config.end(); it ++){
		_locations.push_back(setLocations(*it));
	}
}



//Tries to create a new Socket(for Server), bind and listen in order to all be setup
//Loop logic to modify a bit, check ip before loop
//if error log then continue, else pushback();
//if _sockets empty , end server 
int Server::startServer(){
	std::cout << "Starting new server named : " << _name << std::endl;
	for(std::vector<int>::iterator it = _ports.begin(); it != _ports.end(); it ++){

		int newSock = socket(AF_INET, SOCK_STREAM, 0);
		int temp = 1;
		_socketAddress.sin_port = htons(*it);
		if (newSock < 0){
			Log(_name+": Can't create newSocket");
			continue;
		}
		if(_ipAdrs.empty()||inet_pton(AF_INET, _ipAdrs.c_str(), &_socketAddress.sin_addr) <= 0){
			close(newSock);
			Log(_name+": Invalid IP address");
			std::cout << _ipAdrs << std::endl;
			continue;
		}
		if (setsockopt(newSock, SOL_SOCKET, SO_REUSEADDR, &temp, sizeof(int)) == -1) {
   			Log(_name + ": Unable to use socket options, if connexion failed ports will be still in use");
		}
		if (bind(newSock, (struct sockaddr *)&_socketAddress, _socketLen) < 0){
			close(newSock);
			Log(_name+": Can't bind socket to address");
			continue;
		}
		if (listen(newSock, 10) < 0){
			close(newSock);
			Log(_name+": Socket failed to listen");
			continue;
		}
		else
			_sockets.push_back(newSock);
	}
	if(_root.empty())
		return(ReturnWithMessage(1, _name + ": No root define"));
	if(_sockets.empty())
		return(ReturnWithMessage(1, _name+": Failure not avaible ports"));
	return (ReturnWithMessage(0, _name+": Succes binding sockets to server"));
}

//Close the Socket
void Server::endServer(){
	for(std::vector<int>::iterator it = _sockets.begin(); it != _sockets.end(); it++)
		close(*it);
}

std::string const &Server::getName()const{
	return _name;
}

std::string const &Server::getIp()const{
	return _ipAdrs;
}

std::string const &Server::getRoot()const{
	return _root;
}


std::string const &Server::getphpCgi()const{
	return _php_cgi_path;
}

bool Server::getSendfile()const{
	return _sendfile;
}

int Server::getBodySize()const{
	return _client_max_body_size;
}

unsigned int Server::getSocketLen()const{
	return _socketLen;
}

void Server::printServ(){
	Log("----------------Printing server-------------");
	Log("SERVER NAME " + _name);
	Log("IP ADRS : " + _ipAdrs);
	Log("Ports : ");
	for (std::vector<int>::iterator it = _ports.begin(); it != _ports.end(); it ++)
		std::cout << *it << std::endl;
	Log("TEST WITH LOCATIONS, any vector should be the same tough");
	for(std::vector<LocationConfig>::iterator it = _locations.begin(); it != _locations.end(); it ++){
		std::cout << it->_location_match <<std::endl;
		for(std::vector<LocationConfig>::iterator nested = it->_nested_locations.begin(); nested != it->_nested_locations.end(); nested++)
			std::cout << "		" << nested->_location_match << std::endl;
	}
}