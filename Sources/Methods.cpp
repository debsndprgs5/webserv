#include "Methods.hpp"


//setup client and default errors, do the method aked by client
//doMethod changes the variable ->response(the full message send back to internet)
Methods::Methods(Client *client){
    _client = client;
    _defaultErrors["500"] = "../defaultErrors/500.html"//Internal Server Error
	_defaultErrors["400"] = "../defaultErrors/400.html";//BadRequest
	_defaultErrors["404"] = "../defaultErrors/404.html";//NotFound
	_defaultErrors["405"] = "../defaultErrors/405.html";//NotImplemented 
    _defaultErrors["409"] = "../defaultErrors/409.html";//File allready exist
	_allowedTypes[".php"] = "application/php";
    _allowedTypes[".js"] = "application/javascript";
    _allowedTypes[".html"] = "text/html";
    _allowedTypes[".htm"] = "text/html";
    _allowedTypes[".css"] = "text/css";
    _allowedTypes[".jpeg"] = "image/jpeg";
    _allowedTypes[".jpg"] = "image/jpeg";
    _allowedTypes[".gif"] = "image/gif";
    _allowedTypes[".png"] = "image/png";
    _allowedTypes[".txt"] = "text/plain"
	doMethod();
}

Methods::~Methods(){

}

std::string &Methods::getResponse(){
    return _response;
}

bool Methods::IsMethodAllowed(std::vector<std::string> Allowed, std::string method){
    for(std::vector<std::string>::iterator it = Allowed.begin(); it != Allowed.end(); it ++){
        if(*it == method)
            return true;
    }
    return false;
}


//If no trigger -> regluar GET or DELETE -> just search in Loc for path
//If trigger -> POST request -> split last part to find the rigth location to add stuff 
std::string Methods::ParsedUri(int trigger){
    std::string path 
    if (trigger == 1) {
        size_t lastSlash = _parsedRequest.uri.find_last_of('/');

        if (lastSlash != std::string::npos) {
            // Store the last part of the URI (after the last '/')
            _safePost = _parsedRequest.uri.substr(lastSlash + 1);
            // Remove the last part from the URI path to keep only the parent directory
            path = _client->_server->getRoot() + "/" + _parsedRequest.uri.substr(0, lastSlash);
        } else {
            // If no '/' is found, set _safePost to the entire URI and return only the root
            _safePost = _parsedRequest.uri;
            path = _client->_server->getRoot();
        }
    }
    else
     std::string path = _client->_server->getRoot() +"/"+ _parsedRequest.uri;
	return path;
}

//Process all the request , set ret and response accordingly
void Methods::doMethod(){

    LocationConfig conf;
    if(-parsedRequest.method == "POST"){
        conf = findConfig(parsedUri(1), _client->_server->_locations);
        if(conf == NULL)
            fillError(404);//notFound
        else
            myPost(findConfig(parsedUri(1), _client->_server->_locations), parsedUri(1));
    }
    conf = findConfig(parsedUri(0), _client->_server->_locations);
    else if(_parsedRequest.method == "GET")
         myPost(findConfig(parsedUri(0), _client->_server->_locations), parsedUri(0));
    else if(_parsedRequest.method == "DELETE")
        myDelete(findConfig(parsedUri(0), _client->_server->_locations), parsedUri(0));
    else
        fillError(404);//Method not implemented 
}

//Need iterate trough Locations finding for Path, spliting each /
// Returns the found locationConfig to access all option from specific loc
LocationConfig* Methods::findConfig(const std::string& requestedPath, std::vector<LocationConfig>& locations) {
    std::string delimiter = "/";
    size_t pos = requestedPath.find(delimiter);
    std::string currentPart = requestedPath.substr(0, pos);
    std::string remainingPath = (pos == std::string::npos) ? "" : requestedPath.substr(pos + 1);

    for (size_t i = 0; i < locations.size(); ++i) {
        if (locations[i].path == currentPart) {
            if (remainingPath.empty()) {
                return &locations[i]; // Return the pointer to modify the instance
            }
            LocationConfig* foundConfig = findNestedPath(remainingPath, locations[i].nestedLocations);
            if (foundConfig != NULL) {
                return foundConfig; // Return the instance found deeper in recursion
            }
        }
    }
    return NULL; // Return nullptr if no match is found
}


//Fill Response with content of path
//open path put what's inside in responseBody
//Set _response accordingly
void Methods::myGet(LocationConfig Conf, std::string path){

	if(IsMethodAllowed(_client->_server.allowed, _parsedRequest.method) == true && IsMethodAllowed(Conf.allowed, _parsedRequest.method) == true)
    {
        std::ifstream file(path.c_str()); // Open the file at the given path
        if (file.is_open()) {
            std::ostringstream contentStream;
            contentStream << file.rdbuf(); // Stream the entire file content into the string
            file.close();
            _content = contentStream.str(); // Get the string content of the file
            ret = 200; // OK
            setResponse();
            return;
        }
         else
            fillError(404); // Not Found
    }
    fillError(400);//Methods Not allowed
}

//Add to path the body in serverfiles
void Methods::myPost(LocationConfig Conf, std::string path){
	
	if(IsMethodAllowed(_client->_server.allowed, _parsedRequest.method) == true && IsMethodAllowed(Conf.allowed, _parsedRequest.method) == true)
	{
        std::ifstream testPath(path.c_str()); // Test if path exists
        if (!testPath) {
            fillError(404); //not found
            return;
        }
        testPath.close();
        size_t lastDot = _safePost.find_last_of('.');
        if (lastDot == std::string::npos) {
            fillError(400); // No extension found (Bad Request)
            return;
        }
        std::string ext = _safePost.substr(lastDot + 1); // Extract extension
        if (_allowedTypes.find(ext) == _allowedTypes.end()) {
            fillError(400); // Invalid extension (Bad Request)
            return;
        }
        std::string fullFilePath = path + "/" + _safePost;
        std::ifstream existingFile(fullFilePath.c_str());
        if (existingFile.is_open()) {
            existingFile.close();
            fillError(409); //File already exists
            return;
        }
        std::ofstream newFile(fullFilePath.c_str());
        if (newFile) {
            newFile.close();
            ret = 201;
            setResponse();
            return;
        } else {
            fillError(500); // Internal Server Error
            return;
        }

	}
	fillError(400);//Methods Not allowed
}

//Rm from path in serverfiles
void Method::myDelete(LocationConfig Conf, std::string path){
	
	if(IsMethodAllowed(_client->_server.allowed, _parsedRequest.method) == true && IsMethodAllowed(Conf.allowed, _parsedRequest.method) == true){
        std::ifstream file(path.c_str());
        if (file.is_open()) {
            file.close(); // Close the file since it exists
            if (std::remove(path.c_str()) == 0) {
                ret = 200; // File successfully deleted
                setResponse();
                return;
            }
            else {
                fillError(500); // Internal Server Error: Deletion failed
                return;
            }
        } 
        else 
            fillError(404); // Not Found: File does not exist
            
	}
	fillError(400);//Methods Not allowed
}



//Fill response with error_code
//Check in server for error_code if none found uses it's mapError, opens path
//Put what's inside in _response
void Method::fillError(std::string error_code){

    std::string errorPagePath;
    if (_client->_server->_errorpages.find(errorCode) != _client->_server->_errorpages.end())//error code set in conf
        errorPagePath = _client->_server->_errorpages[errorCode];
    else if (_defaultError.find(errorCode) != _defaultError.end())//error code found in default
        errorPagePath = _defaultError[errorCode];
    else {
        _content = "<html><body><h1>Error " + errorCode + "</h1><p>An error occurred.</p></body></html>";
        ret = std::atoi(errorCode.c_str()); // Set the HTTP response code
        Log("Unknow ERROR CODE, shoudln't happend code = " + error_code);
        return;
    }
    std::ifstream errorFile(errorPagePath.c_str());
    if (errorFile.is_open()) {
        std::ostringstream contentStream;
        contentStream << errorFile.rdbuf(); // Read the file content
        errorFile.close();

        _content = contentStream.str(); // Set the response body
        ret = std::atoi(errorCode.c_str());
    }
    else {
        // If the error file can't be opened, fallback to a generic error message
        _content = "<html><body><h1>Error " + errorCode + "</h1><p>Unable to load the error page.</p></body></html>";
        ret = std::atoi(errorCode.c_str());
    }
    setResponse();
}

void Methods::setResponse(){
    if(_responseBody)
        _response = "ResponseBody : " + _responseBody + "\n";
    if(_content)
        _response += "Content : " + _content + "\n";
    else if (_responseBody.empty() && _content.empty())
        _response = "FULL SHIT BRO WHY IT'S FULL EMPTY"
}