#include "../Includes/Methods.hpp"


//setup client and default errors, do the method aked by client
//doMethod changes the variable ->response(the full message send back to internet)
Methods::Methods(Client *client, HttpRequest parsedRequest){
    _client = client;
    _parsedRequest = parsedRequest;
    _defaultErrors["500"] = "defaultErrors/500.html";//Internal Server Error
	_defaultErrors["400"] = "defaultErrors/400.html";//BadRequest
	_defaultErrors["404"] = "defaultErrors/404.html";//NotFound
	_defaultErrors["405"] = "defaultErrors/405.html";//NotImplemented 
    _defaultErrors["409"] = "defaultErrors/409.html";//File allready exist
	_allowedTypes[".php"] = "application/php";
    _allowedTypes[".js"] = "application/javascript";
    _allowedTypes[".html"] = "text/html";
    _allowedTypes[".htm"] = "text/html";
    _allowedTypes[".css"] = "text/css";
    _allowedTypes[".jpeg"] = "image/jpeg";
    _allowedTypes[".jpg"] = "image/jpeg";
    _allowedTypes[".gif"] = "image/gif";
    _allowedTypes[".png"] = "image/png";
    _allowedTypes[".txt"] = "text/plain";
	Log("ICI");
    handleRequest();
	Log("LABAS");
	doMethod();
	Log("PARLA");
}

Methods::~Methods(){

}

std::string &Methods::getResponse(){
    return _response;
}

bool Methods::isMethodAllowed(std::vector<std::string> Allowed, std::string method){
    for(std::vector<std::string>::iterator it = Allowed.begin(); it != Allowed.end(); it ++){
        if(*it == method)
            return true;
    }
    return false;
}

//Set all variable check if method is allowed
void Methods::handleRequest(){
	std::string searchLocationPath;
	LocationConfig *config;
	Log("SEARCH PAST");
    searchLocationPath = findLocationPath(_parsedRequest.uri);
	Log("BEFORE FINDCONFIG LOOP");
    config = findConfig(searchLocationPath, _client->_server->_locations);
	Log("AFTER   ");
	if(config != NULL)
		setConfig(config);
	else 	
		setConfig();
	Log("handleRequestEND");
	
}


//Cut if more then one slash , assuming no error in uri
std::string Methods::findLocationPath(std::string uri){
	std::string searchPath;
	Log("string = NULL ?");
	size_t lastSlash = uri.find_last_of('/');
	Log("FIND LAST OF");
	if(lastSlash == 0)
		return ("");
	Log("lastSlash == 0");
	if(lastSlash != std::string::npos)
		searchPath = uri.substr(0, lastSlash);
	return("");
}


//Needs to add aliases
void Methods::setConfig(){
	_root = _client->_server->getRoot();
	_methods = _client->_server->_methods;
	_download_dir = _client->_server->getDownloadDir();
	_php_cgi_path = _client->_server->getphpCgi();
}

void Methods::setConfig(LocationConfig *config){
	_root = config->_root;
	_methods = config->_methods;
	_download_dir = config->_download_dir;
	_php_cgi_path = config->_php_cgi_path;
}

//a tester 
LocationConfig *Methods::findConfig(std::string path, std::vector<LocationConfig> &locations){
    if (path.empty()) {
        // Path is empty (e.g., GET /index.html), return NULL to use root and aliases.
        return NULL;
    }
    // Split the path into segments by '/'
    std::vector<std::string> segments;
    std::string delimiter = "/";
    size_t start = 0;
    size_t end = path.find(delimiter);
    while (end != std::string::npos) {
        segments.push_back(path.substr(start, end - start));
        start = end + 1;
        end = path.find(delimiter, start);
    }
    // Add the last segment (if any)
    if (start < path.size()) {
        segments.push_back(path.substr(start));
    }

    // Begin correlation with LocationConfig objects
    std::string currentPath = "/";
    for (size_t i = 0; i < segments.size(); ++i) {
        for (size_t j = 0; j < locations.size(); ++j) {
            if (locations[j]._location_match == segments[i]) {
                if (i == segments.size() - 1) {
                    // Found a match and no more segments left
                    return &locations[j];
                } else if (!locations[j]._nested_locations.empty()) {
                    // Recurse into nested locations
                    return findConfig(path.substr(path.find(segments[i]) + segments[i].length() + 1), locations[j]._nested_locations);
                }
            }
        }
    }
    Log("FOR TEST THIS SHOUDLN'T APPEAR ?");
    // No matching location found
    return NULL;
}

void Methods::doMethod(){
	if(isMethodAllowed(_methods, _parsedRequest.method) == true){
		if(_parsedRequest.method == "POST")
			myPost();
		if(_parsedRequest.method == "GET")
			myGet();
		if(_parsedRequest.method == "DELETE")
			myDelete();
		else
			fillError("501");//Not implemented
	}
	fillError("405");//Not allowed 
}

void Methods::myPost(){
	size_t lastSlash = _parsedRequest.uri.find_last_of('/');
	std::string path = _root + _parsedRequest.uri.substr(0, lastSlash);
	std::string safePost = _parsedRequest.uri.substr(lastSlash+1);
	std::ifstream testPath(path.c_str()); // Test if path exists
	if (!testPath) {
		fillError("404"); //not found
		return;
	}
	testPath.close();
	size_t lastDot = safePost.find_last_of('.');
	if (lastDot == std::string::npos) {
		fillError("400"); // No extension found (Bad Request)
		return;
	}
	std::string ext = safePost.substr(lastDot + 1); // Extract extension
	if (_allowedTypes.find(ext) == _allowedTypes.end()) {
		fillError("400"); // Invalid extension (Bad Request)
		return;
	}
	std::string fullFilePath = path + "/" + safePost;
	std::ifstream existingFile(fullFilePath.c_str());
	if (existingFile.is_open()) {
		existingFile.close();
		fillError("500"); //File already exists
		return;
	}
	std::ofstream newFile(fullFilePath.c_str());
	if (newFile) {
		newFile.close();
		_ret = 201;
		setResponse();
		return;
	} else {
		fillError("500"); // Internal Server Error
		return;
	}

}

void Methods::myGet(){
	std::string path;
	path = _root + _parsedRequest.uri;//Needs to do aliases
	std::ifstream file(path.c_str()); // Open the file at the given path
	if (file.is_open()) {
		std::ostringstream contentStream;
		contentStream << file.rdbuf(); // Stream the entire file content into the string
		file.close();
		_content = contentStream.str(); // Get the string content of the file
		Log("CONTENT : " + _content);
		_ret = 200; // OK
		setResponse();
		return;
	}
	else
		fillError("404"); // Not Found
}

void Methods::myDelete(){

	std::string path;
	path = _root + _parsedRequest.uri;//Needs to check for aliases 
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

//Fill response with error_code
//Check in server for error_code if none found uses it's mapError, opens path
//Put what's inside in _response
void Methods::fillError(std::string error_code){

    std::string errorPagePath;
    if (_client->_server->_error_page.find(error_code) != _client->_server->_error_page.end())//error code set in conf
        errorPagePath = _client->_server->_error_page[error_code];
    else if (_defaultErrors.find(error_code) != _defaultErrors.end())//error code found in default
        errorPagePath = _defaultErrors[error_code];
    else {
        _content = "<html><body><h1>Error " + error_code + "</h1><p>An error occurred.</p></body></html>";
        _ret = std::atoi(error_code.c_str()); // Set the HTTP response code
        Log("Unknow ERROR CODE, shoudln't happend code = " + error_code);
        return;
    }
    std::ifstream errorFile(errorPagePath.c_str());
    if (errorFile.is_open()) {
        std::ostringstream contentStream;
        contentStream << errorFile.rdbuf(); // Read the file content
        errorFile.close();

        _content = contentStream.str(); // Set the response body
        _ret = std::atoi(error_code.c_str());
    }
    else {
        // If the error file can't be opened, fallback to a generic error message
        _content = "<html><body><h1>Error " + error_code + "</h1><p>Unable to load the error page.</p></body></html>" + "DEBUG \n"+ errorPagePath;
        _ret = std::atoi(error_code.c_str());
    }
    setResponse();
}

void Methods::setResponse(){
    if(!_responseBody.empty())
        _response = "ResponseBody : " + _responseBody + "\n";
    if(!_content.empty())
        _response += "Content : " + _content + "\n";
    else if (_responseBody.empty() && _content.empty())
        _response = "FULL SHIT BRO WHY IT'S FULL EMPTY";
    _response += "RET : " + _ret;
}