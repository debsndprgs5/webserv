#include "../Includes/Methods.hpp"

bool Methods::isMethodAllowed(std::vector<std::string> Allowed, std::string method){
    for(std::vector<std::string>::iterator it = Allowed.begin(); it != Allowed.end(); it ++){
        if(*it == method)
            return true;
    }
    return false;
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

std::string getCleanUri(std::string uri){
	std::string cleanUri = uri;
	size_t trigger = cleanUri.find_first_of('?');
	if(trigger != std::string::npos)
		cleanUri.substr(trigger);
	return (cleanUri);
}

std::string Methods::findPath(){
	std::string cleanUri = getCleanUri(_parsedRequest.uri);
	if(_pathWithAlias.empty())
		return (_root+ cleanUri);
	std::string path = _root;
	size_t firstSlash = _pathWithAlias.find_first_of('/');
	if(firstSlash == 0)
		path += _pathWithAlias;
	else 
		path += "/" + _pathWithAlias;
	size_t lastSlash = cleanUri.find_last_of('/');
	if(lastSlash != std::string::npos)
		path += cleanUri.substr(lastSlash);
	return (path);
}

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
            if (locations[j]._location_match == segments[i]) {
                if (i == segments.size() - 1) {
					if(!locations[j]._alias.empty()){
						_pathWithAlias += "/" + locations[j]._alias;
					}
					else
						_pathWithAlias += locations[j]._location_match;
                    return &locations[j];
                } else if (!locations[j]._nested_locations.empty()) {
                    if(!locations[j]._alias.empty()){
						_pathWithAlias += "/" + locations[j]._alias;
					}
					else
						_pathWithAlias += locations[j]._location_match;
                    return findConfig(path.substr(path.find(segments[i]) + segments[i].length() + 1), locations[j]._nested_locations);
                }
            }
        }
    }
    // No matching location found
    return NULL;
}

