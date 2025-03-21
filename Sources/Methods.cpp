#include "../Includes/Methods.hpp"


//setup client and default errors, do the method aked by client
//doMethod changes the variable ->response(the full message send back to internet)
Methods::Methods(Client *client, HttpRequest parsedRequest){
	Log("WHY POST DON'T come HERE BIATCH ??");
    _client = client;
    _parsedRequest = parsedRequest;
	_defaultErrors["404"] = "defaultErrors/404.html";//NotFound
	_defaultErrors["405"] = "defaultErrors/405.html";//NotImplemented 
	_defaultErrors["413"] = "defaultErrors/413.htlm";//Payload exceeds max_body_size.
    _defaultErrors["500"] = "defaultErrors/500.html";//Internal Server Error
    _defaultErrors["501"] = "defaultErrors/501.html";//Not Implemented
	_allowedTypes[".php"] = "application/php";
    _allowedTypes[".js"] = "application/javascript";
    _allowedTypes[".html"] = "text/html";
    _allowedTypes[".htlm"] = "text/html";
    _allowedTypes[".css"] = "text/css";
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
		size_t lastSlash = _parsedRequest.uri.find_last_of('/');
		if(lastSlash == _parsedRequest.uri.size()-1){
			if(_parsedRequest.method == "GET")
				_parsedRequest.uri += "index.html";
			else
			{
				fillError("405");
				return;
			}
		}
		Log("___REQUESTED URI :"+_parsedRequest.uri);
    	handleRequest();
		doMethod();
	}
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
    searchLocationPath = findLocationPath(_parsedRequest.uri);
	Log("LOCATION PATH :"+searchLocationPath);
    config = findConfig(searchLocationPath, _client->_server->_locations);
	if(config != NULL){
		setConfig(config);
		Log("CONFIG FOUND");
	}
	else {
		setConfig();
		Log("NO CONFIG FOUND ");
	}
}


//Cut if more then one slash , assuming no error in uri
std::string Methods::findLocationPath(std::string uri){
	std::string searchPath;
	size_t lastSlash = uri.find_last_of('/');
	if(lastSlash == 0)
		return ("");
	if(lastSlash != std::string::npos){
		searchPath = uri.substr(0, lastSlash);
		return searchPath;
	}
	return("");
}


//Needs to add aliases
void Methods::setConfig(){
	_root = _client->_server->getRoot();
	_methods = _client->_server->_methods;;
	_php_cgi_path = _client->_server->getphpCgi();
}

void Methods::setConfig(LocationConfig *config){
	_root = config->_root;
	_methods = config->_methods;
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

	// Add a leading "/" to the first segment
	if (start == 0 && path[0] == '/') {
		start = 1;  // Skip the first "/" since we are going to add it back manually
	}

	while (end != std::string::npos) {
		// Add the "/" to the start of each segment
		segments.push_back("/" + path.substr(start, end - start));
		start = end + 1;  // Move past the delimiter
		end = path.find(delimiter, start);  // Find the next delimiter
	}
	// Add the last segment (if any), with the leading "/"
	if (start < path.size()) {
		segments.push_back("/" + path.substr(start));
	}
    // Begin correlation with LocationConfig objects
    std::string currentPath = "/";
    for (size_t i = 0; i < segments.size(); ++i) {
        for (size_t j = 0; j < locations.size(); ++j) {
			Log("SEGEMENT :"+ segments[i]);
			Log("LOCATION MATCH:" + locations[j]._location_match);
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
    // No matching location found
    return NULL;
}

void Methods::doMethod(){
	std::cout << "\ntest printf : " << _parsedRequest.method << "\n";
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

/* POST HANDLING TEST */

std::string Rtrim(const std::string &s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
        return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// Extraction du boundary à partir du header Content-Type
std::string extractBoundary(const std::string& contentType)
{
    std::string search = "boundary=";
    size_t pos = contentType.find(search);
    if (pos != std::string::npos) {
        std::string boundary = contentType.substr(pos + search.length());
        boundary = Rtrim(boundary);
        // Supprimer d'éventuels guillemets
        if (!boundary.empty() && boundary.front() == '"' && boundary.back() == '"') {
            boundary = boundary.substr(1, boundary.size() - 2);
        }
        return boundary;
    }
    return "";
}

// Découpe le body en utilisant le boundary
std::vector<std::string> splitBodyByBoundary(const std::string& body, const std::string& boundary)
{
    std::vector<std::string> parts;
    std::string delim = "--" + boundary;
    size_t start = 0;
    size_t end = body.find(delim, start);
    while (end != std::string::npos) {
        std::string part = body.substr(start, end - start);
        if (!part.empty())
            parts.push_back(part);
        start = end + delim.length();
        end = body.find(delim, start);
    }
    return parts;
}

// Vérifie si une partie est celle contenant un fichier
bool isFilePart(const std::string& part)
{
    return part.find("filename=") != std::string::npos;
}

// Extraction du nom de fichier à partir de la partie
std::string extractFileName(const std::string& part)
{
    size_t pos = part.find("filename=\"");
    if (pos == std::string::npos)
        return "";
    pos += 10; // longueur de 'filename="'
    size_t endPos = part.find("\"", pos);
    if (endPos == std::string::npos)
        return "";
    return part.substr(pos, endPos - pos);
}

// Extraction du contenu du fichier à partir de la partie
std::string extractFileContent(const std::string& part)
{
    // Trouver la séquence "\r\n\r\n" qui sépare les headers du contenu
    size_t pos = part.find("\r\n\r\n");
    if (pos == std::string::npos)
        return "";
    pos += 4; // passer la séquence de séparation
    // Optionnel : retirer le dernier CRLF qui précède le prochain boundary
    size_t endPos = part.rfind("\r\n");
    if (endPos == std::string::npos)
        return part.substr(pos);
    return part.substr(pos, endPos - pos);
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
                _response = "Fichier '" + fileName + "' téléchargé avec succès.";
                return;
            } else {
                fillError("500"); // Erreur serveur interne
                return;
            }
        }
    }
    // Si aucune partie fichier n'a été trouvée
    fillError("400");
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

std::string Methods::findWhat(){
	if(_mappedCodes.find(_ret) != _mappedCodes.end())
		return (_mappedCodes[_ret]);
	return("");
}

std::string Methods::findType(std::string uri){
	size_t lastDot = uri.find_last_of('.');
	if(_ret != 200 && _ret != 201)
		return("text/html");
	if(lastDot  == std::string::npos)
		return("");//no extention
	std::string extention = uri.substr(lastDot);
	if(_allowedTypes.find(extention) != _allowedTypes.end())
		return(_allowedTypes[extention]);
	return("");
}

void Methods::setResponse(){
	std::string what = findWhat();//like "OK" or "Not Found"
	std::string type = findType(_parsedRequest.uri);//like .php .html
	Log("_____BEFORE CREATING RESPONSE ___________\n\n");
	Log("_____WHAT :" + what);
	Log("_____TYPE :" + type);
	Log("_____CONTENT\n" + _content);
	_response = buildHttpResponse(_content, _ret, what, _client->_server->getName(), type);

}