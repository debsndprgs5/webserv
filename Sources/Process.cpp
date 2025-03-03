#include "../Includes/Process.hpp"
static bool run;

//catch ctrl C free the process, anything setting up run on false MUST call freeProcess()
void sigHandler(int sig){
	std::cout << "\nSignal : " << sig << " detected, closing webserv"<< std::endl;
	run = false;
}

Process::Process(){}

Process::~Process(){
	std::cout << "Ending Process"<<std::endl;
	freeProcess();
}

//Accept a socket for new client and says hi to it
void Process::acceptNewClient(struct pollfd &it, std::vector<struct pollfd> &pendingClients, Server *server){
		int SocketClient = accept(it.fd, NULL, NULL);
		if (SocketClient < 0){
			std::cerr << "Error accepting connection"<< std::endl;
			return ;
		}
		Client *current = new Client(SocketClient, server);
		struct pollfd fds;
		fds.fd= current->getSocketClient();
		fds.events = POLLIN;
		pendingClients.push_back(fds);
		if(_MappedClient.find(current->getSocketClient()) == _MappedClient.end()){
			_MappedClient[fds.fd] = current;
		}
		if(_MappedServ.find(it.fd) != _MappedServ.end()){
			std::string ServName= _MappedServ[it.fd]->getName();
			Log(ServName + " : New connection detected...");
		}
}	

bool Process::isPendingDeco(struct pollfd &current, std::vector<struct pollfd> &pendingDeco){
 for (std::vector<struct pollfd>::const_iterator it = pendingDeco.begin(); it != pendingDeco.end(); ++it) {
        if (it->fd == current.fd) {
            return true;
        }
    }
    return false;
}

void Process::handleData(struct pollfd &it, std::vector<struct pollfd> &pendingDeco){

	char buffer[1024] = {0};
	int bytesRecv = recv(it.fd, buffer, 1024, 0);
	if(bytesRecv < 0){
		std::cout << bytesRecv << std::endl;
		Log("Error or client disconnected while receiving data");
	}
	else if(bytesRecv == 0){
		Log("client disconected");
		struct pollfd tmp;
		tmp.fd = it.fd;
		pendingDeco.push_back(tmp);
	}
	else{
		std::cout << "Received from client : " << buffer <<std::endl;
		send(it.fd, RESPONSE.c_str(), RESPONSE.length(), 0);
		// for(std::map<int, *Client>::iterator itMap = _MappedClient.begin(); itMap != _MappedClient.end(); itMap++){
		// 	if(itMap->first == it.fd) client found in client_data_base
		// 		if(it.Map->second.fillRequest()) request added to client and if full 
		//			proccessData(); HTTP dialogue
		// 
		//}
	}
}

//Call poll on _FdArray, if POLLIN is recived : new connection, 
void Process::mainLoop(){
	run = true;
	signal(SIGINT, sigHandler);//Allow exit with ctrlC
	std::vector<struct pollfd> pendingClients;//Tracks of the new clients for outside loop
	std::vector<struct pollfd> pendingDeco;//fd_array that need to be remove outside loop
	while(run == true){
		int resPoll = poll(_fdArray.data(), _fdArray.size(), -1);
		if(resPoll < 0){
			freeProcess();
			if(run == true)
				ExitWithMessage(1,"poll is doing his best....");
		}
		for (std::vector<struct pollfd>::iterator it = _fdArray.begin(); it != _fdArray.end(); ++it) {
			if (it->revents & POLLIN) {
				std::map<int, Server*>::iterator itServ = _MappedServ.find(it->fd);
				if (itServ != _MappedServ.end()) {
					acceptNewClient(*it, pendingClients, itServ->second);
				} else if (_MappedClient.find(it->fd) != _MappedClient.end() && !isPendingDeco(*it, pendingDeco)) {
					handleData(*it, pendingDeco);
				}
			}
		}
		for(std::vector<struct pollfd>::iterator it= pendingDeco.begin(); it != pendingDeco.end(); it++){
			close(it->fd);
			delete _MappedClient[it->fd];
			_MappedClient.erase(it->fd);
		//	delete pendingDeco[it->fd];
		//	_fdArray.erase(it);
		}
	_fdArray.insert(_fdArray.end(), pendingClients.begin(), pendingClients.end());
	pendingClients.clear();
	pendingDeco.clear();
	}
}


// Test the serverS and add it to the containerS, if unvalid server , goes to the next one 
int Process::start(std::vector<ServerConfig> servers){			
	Server* current;	
	for(size_t i=0; i < servers.size(); i++){
		current = new Server(servers[i]);
		int tmp = current->startServer();
		if(tmp != 0)
			delete current;
		else{
			_servArray.push_back(current);
			for(std::vector<int>::iterator it = current->_sockets.begin(); it != current->_sockets.end(); it ++){
			struct pollfd fds;
			fds.fd = *it;
			fds.events = POLLIN;
			_fdArray.push_back(fds);
			_MappedServ[fds.fd] = current;//link the socket sever to the object server
			}

		}
	}
	if (_servArray.empty()){
		freeProcess();
		ExitWithMessage(1,"Unable to process any Servers");
	}
	return 0;
}

void Process::freeProcess() {
	for (std::map<int, Client*>::iterator it = _MappedClient.begin(); it != _MappedClient.end(); it++) {
      	 close(it->first);
		 delete it->second;
    }
	_MappedClient.clear();
    for (std::map<int, Server*>::iterator it = _MappedServ.begin(); it != _MappedServ.end(); it++) {
        it->second->endServer();
		close(it->first);
    }
    _MappedServ.clear(); // Clear the map to remove references
    for (std::vector<Server*>::iterator it = _servArray.begin(); it != _servArray.end(); it++) {
        delete *it;     // Delete the Server object
    }
    _servArray.clear(); // Clear the vector after deallocating memory
}

//Set _Running will stop the poll loop and free all stuff in process
void Process::setRunning(){
	run = false;
	freeProcess();
}