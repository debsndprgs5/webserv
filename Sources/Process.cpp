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
			close(SocketClient);
			Log("Error accepting connection");
			return ;
		}
		if (fcntl(SocketClient, F_SETFL, O_NONBLOCK) < 0){
			close(SocketClient);
			Log("Error setting non-blocking mode");
			return;
		}		
		Client *current = new Client(SocketClient, server);
		struct pollfd fds;
		fds.fd= current->getSocketClient();
		fds.events = POLLIN ;
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

void Process::handleData(struct pollfd &it, std::vector<struct pollfd> &pendingDeco) 
{
	char buffer[1024] = {0};
	int bytesRecv = recv(it.fd, buffer, sizeof(buffer), 0);

    if (bytesRecv < 0)
    {
        Log("Client disconnected");
        struct pollfd tmp;
        tmp.fd = it.fd;
        pendingDeco.push_back(tmp);
    }
	else if(bytesRecv == 0){
		Log("Recve = 0");
		Client* client = _MappedClient[it.fd];
		if(client->getRecveCheck() == false)
			client->setRecveCheck(true);
		else if(client->getRecveCheck() == true){
			Log("Client disconnected");
        	struct pollfd tmp;
       		tmp.fd = it.fd;
       		pendingDeco.push_back(tmp);
		}
	}
    else
    {
        // On récupère le client associé via _MappedClient
        Client* client = _MappedClient[it.fd];
		client->setRecveCheck(false);
        // On ajoute les données reçues au buffer du client
        client->appendRawData(buffer, bytesRecv);
   	    // Si la requête est complète (headers + body selon Content-Length), on la traite
        if (client->requestIsComplete())
        {
            proccessData(client, it.fd, pendingDeco);
            // Une fois traitée, on vide le buffer pour la prochaine requête
            client->clearRawData();
        }
    }
}

int Process::sendCheck(int fd, const char* data, size_t dataLength, size_t bytesSent) {
    if (bytesSent >= dataLength) {
        // All data has been sent
		Client* client = _MappedClient[fd];
        client->setSendTrigger(0);
        return 0;
    }
    int ret_send = send(fd, data + bytesSent, dataLength - bytesSent, 0);
    if (ret_send < 0) {
        Client* client = _MappedClient[fd];
		client->setBytesSend(bytesSent);
		client->setLeftover(std::string (data+bytesSent, dataLength - bytesSent));
		client->setSendTrigger(true);
		Log("LATER RETRY CALLED");
        return -1; // Retry later 
    } else if (ret_send == 0) {
        // Connection lost
        return 0;
    } else {
        // Successfully sent some bytes, recursively send the remaining data
        return sendCheck(fd, data, dataLength, bytesSent + ret_send);
    }
}


void Process::proccessData(Client *client, int fd, std::vector<struct pollfd>& pendingDeco) {
    HttpRequest parsedRequest;
    std::string response;
    bool isGood = parseHttpRequest(client->getRequest(), parsedRequest);
    Methods *met = new Methods(client, parsedRequest);
    if (isGood == true) {
        response = met->getResponse();
    } else {
        met->fillError("404"); // Parsing error?
        response = met->getResponse();
    }
    // Send the response in chunks using sendCheck
    int isSent = sendCheck(fd, response.c_str(), response.length());
    if (isSent == 0) {
        Log("Failed to send data to client");
        struct pollfd tmp;
        tmp.fd = fd;
        pendingDeco.push_back(tmp); // Mark client for disconnection
    }
    delete met;
}

//Call poll on _FdArray, if POLLIN is recived : new connection, 
void Process::mainLoop(){
	run = true;
	signal(SIGINT, sigHandler);//Allow exit with ctrlC
	std::vector<struct pollfd> pendingClients;//Tracks of the new clients for outside loop
	std::vector<struct pollfd> pendingDeco;//fd_array that need to be remove outside loop
	while(run == true){
		int resPoll = poll(_fdArray.data(), _fdArray.size(), 10);
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
				}
				else if (_MappedClient.find(it->fd) != _MappedClient.end() && !isPendingDeco(*it, pendingDeco))
				{
					Client* cur = _MappedClient[it->fd];
					if(cur->getSendTrigger() == true){
						std::string leftover = cur->getLeftover();
						int success = sendCheck(it->fd, leftover.c_str(), leftover.length(), cur->getBytesSend());
						if(!success){
							struct pollfd tmp;
            				tmp.fd = it->fd;
            				pendingDeco.push_back(tmp);
        				}
					}
					else
						handleData(*it, pendingDeco);
				}
			}
			else if (it->revents & (POLLERR | POLLHUP | POLLNVAL)) {
  			  Log("Erreur ou déconnexion détectée sur la socket");
  			  pendingDeco.push_back(*it);
  			  continue;
			}
		}
		for(std::vector<struct pollfd>::iterator it= pendingDeco.begin(); it != pendingDeco.end(); it++) {
    close(it->fd);
    
    // Détruire le client s’il existe dans la map
    std::map<int, Client*>::iterator found = _MappedClient.find(it->fd);
    if (found != _MappedClient.end()) {
        delete found->second;  // delete l'instance
        _MappedClient.erase(found);
    }

    // Ensuite, retirer ce FD de _fdArray
    for (std::vector<struct pollfd>::iterator fdIt = _fdArray.begin(); fdIt != _fdArray.end(); ++fdIt) {
        if (fdIt->fd == it->fd) {
            _fdArray.erase(fdIt);
            break;
        }
    }

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