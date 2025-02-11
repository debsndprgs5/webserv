#include "../Includes/Client.hpp"

Client::Client(){

}

Client::Client(int socket){
    _ClientSocket = socket;
}

Client::~Client(){
    Log("²²²Closing client ....");
    close(_ClientSocket);
}

void Client::setSocketClient(int socket){
    _ClientSocket = socket;
}

int Client::getSocketClient(){
    return _ClientSocket;
}