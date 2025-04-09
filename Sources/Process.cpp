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


int drainCgiPipe(Client* client)
{
	while (true)
	{
		char buffer[1024];
		ssize_t bytesRead = read(client->getCgiPipe(), buffer, sizeof(buffer));

		if (bytesRead > 0) {
			// Add data read to client buffer
			client->appendCgiOutput(std::string(buffer, bytesRead));
			//Keep the loop going for as long as there is data to read 
		}
		else if (bytesRead == 0) {
			// 0 => EOF : CGI is over
			client->cgiHasFinished = true;
			break;
		}
		else {
			// bytesRead < 0 => no more data
			// => stop reading now, keep the main loop going
			break;
		}
	}
	return 0;
}


void Process::acceptNewClient(struct pollfd &it, std::vector<struct pollfd> &pendingClients, Server *server){
	int SocketClient = accept(it.fd, NULL, NULL);
	if (SocketClient < 0){
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return;
		}
		Log("Error accepting connection: " + std::string(strerror(errno)));
		return;
	}
	if (fcntl(SocketClient, F_SETFL, O_NONBLOCK) < 0){
		close(SocketClient);
		Log("Error setting non-blocking mode");
		return;
	}

	Client *current = new Client(SocketClient, server);
	struct pollfd fds;
	fds.fd = current->getSocketClient();
	fds.events = POLLIN;
	pendingClients.push_back(fds);

	if(_MappedClient.find(current->getSocketClient()) == _MappedClient.end()){
		_MappedClient[fds.fd] = current;
	}
	if(_MappedServ.find(it.fd) != _MappedServ.end()){
		std::string ServName = _MappedServ[it.fd]->getName();
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
	if(_MappedClient[it.fd]->getCgiPipe() > 0)
		return;
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
		Client* client = _MappedClient[it.fd];
		client->setRecveCheck(false);
		// We add data read to client buffer
		client->appendRawData(buffer, bytesRecv);
		// If request is complete (headers + body according to Content-Length), we process it
		if (client->requestIsComplete())
		{
			proccessData(client, it.fd, pendingDeco);
			// Once data has been  process we clean it
			client->clearRawData();
		}
	}
}

int Process::sendCheck(int fd, const char* data, size_t dataLength, size_t bytesSent) {

   Log("Probleme ici ?");
	if (bytesSent >= dataLength) {
		// All data has been sent
		Client* client = _MappedClient[fd];
		client->setSendTrigger(0);
		return 0;
	}
	int ret_send = send(fd, data + bytesSent, dataLength - bytesSent, 0);
	if (ret_send < 0) {
		Log("SEND RET IS -1");
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
	Methods *met = new Methods(client, parsedRequest, _pendingAdd);
	if(client->getCgiPipe() > 0){
		Log("Pipe found");
		delete met;
		return;
	}
	else if (isGood == true) {
		response = met->getResponse();
	}

	else {
		met->fillError("404"); // Parsing error?
		response = met->getResponse();
	}
	// Send the response in chunks using sendCheck
	int isSent = sendCheck(fd, response.c_str(), response.length());
	if (isSent == 0) {
		struct pollfd tmp;
		tmp.fd = fd;
		pendingDeco.push_back(tmp); // Mark client for disconnection
	}
	delete met;
}

void Process::mainLoop() {
    run = true;
    signal(SIGINT, sigHandler);

    std::vector<struct pollfd> pendingClients;
    std::vector<struct pollfd> pendingDeco;

    while (run) {
        int resPoll = poll(_fdArray.data(), _fdArray.size(), 1000);
        if (resPoll < 0) {
            freeProcess();
            break;
        }

        for (std::vector<struct pollfd>::iterator it = _fdArray.begin(); it != _fdArray.end(); ++it) {
            // 1) Fatal Errors
            if (it->revents & POLLERR) {
                // => on marquera ce fd pour fermeture
                pendingDeco.push_back(*it);
                continue;
            }

            // 2) Socket Serveur ?
            if (_MappedServ.find(it->fd) != _MappedServ.end()) {
                if (it->revents & POLLIN) {
                    // accepter un nouveau client
                    acceptNewClient(*it, pendingClients, _MappedServ[it->fd]);
                }
                continue;
            }

            // 3) Socket Client normal ?
            if (_MappedClient.find(it->fd) != _MappedClient.end()) {
                Client* cur = _MappedClient[it->fd];

                if (it->revents & POLLIN) {
                    // Lire tout ce qui est dispo
                    bool clientClosed = false;
                    while (true) {
                        char buffer[1024];
                        ssize_t bytesRecv = recv(it->fd, buffer, sizeof(buffer), 0);
                        if (bytesRecv > 0) {
                            cur->appendRawData(buffer, bytesRecv);
                        }
                        else if (bytesRecv == 0) {
                            clientClosed = true;
                            break;
                        }
                        else { 
                            // -1 => plus rien à lire maintenant
                            break;
                        }
                    }
                    if (clientClosed) {
                        // Déconnexion
                        struct pollfd tmp;
                        tmp.fd = it->fd;
                        pendingDeco.push_back(tmp);
                    } else {
                        // Traiter la requête si complète
                        if (cur->requestIsComplete()) {
                            proccessData(cur, it->fd, pendingDeco);
                            cur->clearRawData();
                        }
                    }
                }

                // (Si besoin) if (it->revents & POLLOUT) { ... } pour l'envoi asynchrone
                continue;
            }

            // 4) Sinon, est-ce un pipe CGI ?
            int clientFd = isCgiPipe(it->fd);
            if (clientFd) {
                Client* client = _MappedClient[clientFd];

                // Si c’est un pipe CGI, on veut lire en cas de POLLIN **ou** POLLHUP
                // => S’il y a un HUP, on peut quand même avoir un dernier chunk à lire.
                if (it->revents & (POLLIN | POLLHUP)) {
                    drainCgiPipe(client);  // lit tout ce qui est dispo

                    // Si le CGI est terminé (read => 0)
                    if (client->cgiHasFinished) {
                        // Construire la réponse
                        std::string response = client->getResponse(client->getCgiOutput());
                        int sendStatus = sendCheck(client->getSocketClient(),
                                                   response.c_str(),
                                                   response.size());
                        if (sendStatus == 0) {
                            // le client HTTP est fermé
                            struct pollfd tmp;
                            tmp.fd = client->getSocketClient();
                            pendingDeco.push_back(tmp);
                        }
                        // Fermer le pipe
                        pendingDeco.push_back(*it);
                    }
                }
                // Pas de "continue" nécessaire, on est déjà en fin de loop
            }
        } // Fin du for poll

        // Déconnexions
        for (size_t i = 0; i < pendingDeco.size(); i++) {
            int closingFd = pendingDeco[i].fd;
            close(closingFd);

            // Supprimer le client associé
            std::map<int, Client*>::iterator foundClient = _MappedClient.find(closingFd);
            if (foundClient != _MappedClient.end()) {
                delete foundClient->second;
                _MappedClient.erase(foundClient);
            }
            // Retirer ce fd de _fdArray
            for (std::vector<struct pollfd>::iterator fdIt = _fdArray.begin(); fdIt != _fdArray.end(); ++fdIt) {
                if (fdIt->fd == closingFd) {
                    _fdArray.erase(fdIt);
                    break;
                }
            }
        }
        pendingDeco.clear();
		for (size_t i=0; i<_pendingAdd.size(); i++)
		    _fdArray.push_back(_pendingAdd[i]);
		_pendingAdd.clear();

        // Ajout des nouveaux clients
        for (size_t i = 0; i < pendingClients.size(); i++) {
            _fdArray.push_back(pendingClients[i]);
        }
        pendingClients.clear();
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

//Returns client socket/fd
int Process::isCgiPipe(int fd)
{
	for (std::map<int, Client*>::iterator it = _MappedClient.begin(); it != _MappedClient.end(); ++it) {
		if (it->second->getCgiPipe() == fd)
			return it->first;
	}
	return 0;
}
