#include "../Includes/Methods.hpp"




std::string Methods::findWhat(){
	if(_mappedCodes.find(_ret) != _mappedCodes.end())
		return (_mappedCodes[_ret]);
	return("");
}

std::string Methods::findType(std::string uri){
	size_t lastDot = uri.find_last_of('.');
	if(_ret != 200)
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
	_response = buildHttpResponse(_content, _ret, what, _client->_server->getName(), type);

}


std::string getCurrentDate()
{
    std::time_t now = std::time(NULL);
    char buf[100];
    std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", std::gmtime(&now));
    return std::string(buf);
}

// Function to build an HTTP response
std::string buildHttpResponse(const std::string& body, int statusCode, const std::string& statusMessage, const std::string& server_name, const std::string& contentType)
{
    std::ostringstream response;

    // Line of status
    response << "HTTP/1.1 " << statusCode << " " << statusMessage << "\r\n";

    // Headers
    response << "Date: " << getCurrentDate() << "\r\n";
    response << "Server: " << server_name << "\r\n";
    response << "Content-Type: " << contentType << "\r\n";
    response << "Content-Length: " << body.size() << "\r\n";
    response << "Connection: close\r\n"; // or "keep-alive"
    response << "\r\n"; // Empty line to split headers and body

    response << body;

    return response.str();
}
