#include "../Includes/Server.hpp"

//Should do nothing here
Server::Server(){
}

Server::~Server(){
	std::cout << _Name << " ending service ...." << std::endl;
	endServer();
}

//Config all variables according to Config item
Server::Server(Config Conf){
	this->setConfig(Conf);
}

Server::Server(const Server &cpy){
	*this=cpy;
}

Server &Server::operator=(const Server &cpy){
	_Name = cpy._Name;
	_IpAdrss = cpy._IpAdrss;
	_Ports = cpy._Ports;
	_Socket = cpy._Socket;
	_SocketAddress = cpy._SocketAddress;
	_SocketLen = cpy._SocketLen;
	return *this;
}

void Server::setConfig(Config Conf){
	_Name = Conf.getName();
	_IpAdrss = Conf.getIp();
	_Ports = Conf.getPort();
	memset(&_SocketAddress, 0, sizeof(_SocketAddress));
	_SocketLen = sizeof(struct sockaddr_in);
	_SocketAddress.sin_family = AF_INET;
	_SocketAddress.sin_port= htons(_Ports);
	
}


//Tries to create a new Socket(for Server), bind and listen in order to all be setup
int Server::startServer(){
	std::cout << "Starting new server named : " << _Name << std::endl;
	_Socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_Socket < 0){
		return (ReturnWithMessage(1,_Name+": Can't create socket"));
	}
	if(_IpAdrss.empty()||inet_pton(AF_INET, _IpAdrss.c_str(), &_SocketAddress.sin_addr) <= 0){
		close(_Socket);
		return(ReturnWithMessage(1, _Name+": Invalid IP address"));
	}
	if (bind(_Socket, (struct sockaddr *)&_SocketAddress, _SocketLen) < 0){
		close(_Socket);
		perror("BINDING");
		return (ReturnWithMessage(1, _Name+": Can't bind soket to address"));
	}
	if (listen(_Socket, MAX_CLIENTS) < 0){
		close(_Socket);
		perror(": Socket failed to listen");
		return (ReturnWithMessage(1, _Name+": Socket failed to listen"));
	}
	std::ostringstream ss;
    ss << _Name << ": listening on " << _IpAdrss << ":" << _Ports;
	return (ReturnWithMessage(0, ss.str()));
}

//Close the Socket
void Server::endServer(){

	close(_Socket);
}

std::string const &Server::getName()const{
	return _Name;
}

std::string const Server::getIp()const{
	return _IpAdrss;
}

int Server::getSocket()const{
	return _Socket;
}

int Server::getPorts()const{
	return _Ports;
}

unsigned int Server::getSocketLen()const{
	return _SocketLen;
}