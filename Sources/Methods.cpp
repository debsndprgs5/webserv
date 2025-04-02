#include "../Includes/Methods.hpp"
#include "post_tools.cpp"


//setup client and default errors, do the method asked by client
//doMethod changes the variable ->response(the full message send back to internet)
Methods::Methods(Client *client, HttpRequest parsedRequest){
    _client = client;
    _parsedRequest = parsedRequest;

	_defaultErrors["404"] = "defaultErrors/404.html";//NotFound
	_defaultErrors["405"] = "defaultErrors/405.html";//NotAllowed
	_defaultErrors["413"] = "defaultErrors/413.html";//Payload exceeds max_body_size.
    _defaultErrors["500"] = "defaultErrors/500.html";//Internal Server Error
    _defaultErrors["501"] = "defaultErrors/501.html";//Not Implemented
	_allowedTypes[".php"] = "application/php";
	_allowedTypes[".css"] = "text/css";
    _allowedTypes[".html"] = "text/html";
    _allowedTypes[".htlm"] = "text/html";
    _allowedTypes[".jpeg"] = "image/jpeg";
    _allowedTypes[".jpg"] = "image/jpeg";
    _allowedTypes[".gif"] = "image/gif";
    _allowedTypes[".png"] = "image/png";
    _allowedTypes[".txt"] = "text/plain";
	_mappedCodes[200] = "OK";
	_mappedCodes[201] = "Created";
	_mappedCodes[404] = "Not Found";
	_mappedCodes[405] = "Not Implemented";
	_mappedCodes[413] = "Paylods exceeds max_body_size";
	_mappedCodes[500] = "Internal Server Error";
	_mappedCodes[501] = "Not Implemented";
	if(parseHttpRequest(_client->getRequest(), _parsedRequest) == true){
		if(checkPhpCgi() == true){
			cgiHandler();
			return;
		}
		size_t lastSlash = _parsedRequest.uri.find_last_of('/');
		if(lastSlash == _parsedRequest.uri.size()-1){
			if(_parsedRequest.method == "GET")
				_parsedRequest.uri += "index.html";
		}
    	handleRequest();
		doMethod();
	}
}

Methods::~Methods(){
}

std::string &Methods::getResponse(){
    return _response;
}



//Set all variable check if method is allowed
void Methods::handleRequest(){
	std::string searchLocationPath;
	LocationConfig *config;
    searchLocationPath = findLocationPath(_parsedRequest.uri);
    config = findConfig(searchLocationPath, _client->_server->_locations);
	if(config != NULL){
		setConfig(config);
	}
	else {
		setConfig();
	}
}

void Methods::setConfig(){
	_root = _client->_server->getRoot();
	_methods = _client->_server->_methods;;
	_php_cgi_path = _client->_server->getphpCgi();
	_buffer_size = _client->_server->getBodySize();
}

void Methods::setConfig(LocationConfig *config){
	_root = config->_root;
	_methods = config->_methods;
	_php_cgi_path = config->_php_cgi_path;
	_buffer_size = config->_client_body_buffer_size;
}

void Methods::doMethod(){
	if(_client->getRequest().size() > _buffer_size){
		fillError("413");
		return;
	}
	if(isMethodAllowed(_methods, _parsedRequest.method) == true){
		if(_parsedRequest.method == "POST")
			myPost();
		else if(_parsedRequest.method == "GET"){
			myGet();
		}
		else if(_parsedRequest.method == "DELETE")
			myDelete();
		else{
			fillError("501");//Not implemented
		}
	}
	else
		fillError("405");//Not allowed 
}

void Methods::myPost() {
    // Vérifier que le body n'est pas vide
    if (_parsedRequest.body.empty()) {
        fillError("400"); // Bad Request
        return;
    }

    // Extraire le boundary depuis le header Content-Type
    std::string contentType = _parsedRequest.headers["Content-Type"];
    std::string boundary = extractBoundary(contentType);
    if (boundary.empty()) {
        fillError("400"); // Mauvaise requête
        return;
    }

    // Découper le body en parties
    std::vector<std::string> parts = splitBodyByBoundary(_parsedRequest.body, boundary);
    for (size_t i = 0; i < parts.size(); ++i) {
        if (isFilePart(parts[i])) {
            std::string fileName = extractFileName(parts[i]);
            std::string fileContent = extractFileContent(parts[i]);
            if (fileName.empty() || fileContent.empty()) {
                fillError("400"); // Mauvaise requête
                return;
            }

            // Construire le chemin complet pour sauvegarder le fichier
            std::string filePath =  _client->_server->getName() + "/" + fileName;
            std::ofstream outFile(filePath.c_str(), std::ios::binary);
            if (outFile.is_open()) {
                outFile.write(fileContent.c_str(), fileContent.size());
                outFile.close();
                _ret = 201; // Créé
				setResponse();
                return;
            }
            else
            {
                fillError("500"); // Erreur serveur interne
                return;
            }
        }
    }
    // Si aucune partie fichier n'a été trouvée
    fillError("404");
}

bool isExecutable(const char* path) {
    struct stat sb;
	return false;
    if (stat(path, &sb) == 0) {
        // Vérifie les permissions d'exécution pour le propriétaire, le groupe ou les autres
        return (sb.st_mode & S_IXUSR) || (sb.st_mode & S_IXGRP) || (sb.st_mode & S_IXOTH);
    }
    return false; // En cas d'erreur, considère que le fichier n'est pas exécutable
}

bool isHtml(std::string path){
	size_t LastDot = path.find_last_of('.');
	if(LastDot != std::string::npos){
		std::string ext = path.substr(LastDot);
		Log("EXT :" + ext);
		if(ext == ".html")
			return true;
	}
	return false;
}


void Methods::myGet(){
	std::string path;
	path = findPath();//Needs to do aliases
	if(isExecutable(path.c_str()) == true && isHtml(path) == false){
		cgiHandler();
		return;
	}
	std::ifstream file(path.c_str()); // Open the file at the given path
	if (file.is_open()) {
		std::ostringstream contentStream;
		contentStream << file.rdbuf(); // Stream the entire file content into the string
		file.close();
		_content = contentStream.str(); // Get the string content of the file
		_ret = 200; // OK
		setResponse();
		return;
	}
	else
		fillError("404"); // Not Found
}

void Methods::myDelete(){

	std::string path;
	path = findPath();//Needs to check for aliases
	std::ifstream file(path.c_str());
	if (file.is_open()) {
		file.close(); // Close the file since it exists
		if (std::remove(path.c_str()) == 0) {
			_ret = 200; // File successfully deleted
			setResponse();
			return;
		}
		else {
			fillError("500"); // Internal Server Error: Deletion failed
			return;
		}
	} 
	else 
		fillError("404"); // Not Found: File does not exist        
}

