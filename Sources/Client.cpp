#include "../Includes/Client.hpp"

Client::Client(){

}

Client::Client(int socket, Server *server){
    _server = server;
    _ClientSocket = socket;
    Log("----------------Printing server TROUGHT CLIENT-------------");
	Log("SERVER NAME " + _server->getName());
	Log("IP ADRS : " + server->getIp());

}

Client::~Client(){
    Log("²²²Closing client ....");
    close(_ClientSocket);
}

Client &Client::operator=(const Client &cpy){
    _server = cpy._server;
    _ClientSocket = cpy._ClientSocket;
    _request = cpy._request;
    return *this;
}

void Client::setSocketClient(int socket, Server *server){
    _server = server;
    _ClientSocket = socket;
}

int Client::getSocketClient(){
    return _ClientSocket;
}

std::string &Client::getRequest(){
    return _request;
}

//stock buffer from process Return(0)->end not reach, Return(1)->end reach
bool Client::fillRequest(char *buffer){
    _request += std::string(buffer);
    return (isRequestFull());
}

//Check if the method is POST then if it's full, all other request shouldn't be chunked
// true-> Server can respond to client / false -> waiting for full content 
//Commented for easy make, needs to modify httpParser to get the variable
bool Client::isRequestFull(){//Needs to find other way to do this 

   struct HttpRequest Request;
    if(parseHttpRequest(_request, Request) == true){
        if(Request.method == "POST"){
            if(Request._contentLength == Request.body.size())
                return true;
        else 
            return false;
        }
    }
     return true;
}

bool Client::checkEnd(){

    const std::string endSequence = "0\r\n\r\n";
    if (_request.size() >= endSequence.size() &&
        _request.compare(_request.size() - endSequence.size(), endSequence.size(), endSequence) == 0) {
        return true;
    }
    return false;
}

//1rst -> check Method
//2cnd -> if post check if chuncked 


//if Transfer_Enconding:chunked is found in header, the request is uncomplete
//  Then the end of message will content "0\r\n\r\n"