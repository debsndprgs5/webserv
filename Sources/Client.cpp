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

void Client::setSocketClient(int socket, Server *server){
    _server = server;
    _ClientSocket = socket;
}

int Client::getSocketClient(){
    return _ClientSocket;
}

// std::string &Client::getRequest(){
//     return _request;
// }

//stock buffer from process Return(0)->end not reach, Return(1)->end reach
// int Client::fillRequest(char *buffer){
//     _request += buffer.c_str();
//     return (isRequestFull(getRequest()));
// }

// //HTTP magic
// int isRequestFull(){

//     struct HTTPARSE;
//      ICI l'idee c'est d'appeller le parsing HTTP check sa variable Content_length OU Transfer-Encoding: chunked(dans les headers)
//      si on la trouve on sait que la requete est full est on peut y repondre 
//      on retourne 1, le process appelle Method()
// }