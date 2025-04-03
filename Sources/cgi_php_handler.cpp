#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include "../Includes/Methods.hpp"
#include "methodsTools.cpp"




void Methods::setCgiName(){
	int trigger = _parsedRequest.uri.find_first_of('?');
	std::string cleanUri = _parsedRequest.uri;
	if(trigger != std::string::npos)
		cleanUri = _parsedRequest.uri.substr(0, trigger);
	int lastSlash = cleanUri.find_last_of('/');
	if(lastSlash != std::string::npos){
		_cgiName = cleanUri.substr(lastSlash);
	}
	else
		_cgiName = cleanUri;
}

bool Methods::setCgiPath(){
	int trigger = _parsedRequest.uri.find_first_of('?');
	std::string cleanUri = _parsedRequest.uri;
	if(trigger != std::string::npos)
		cleanUri = _parsedRequest.uri.substr(0, trigger);
	std::string searchLocationPath = findLocationPath(cleanUri);
	LocationConfig *config = findConfig(searchLocationPath,  _client->_server->_locations);
	if(config != NULL){
		setConfig(config);
	}
	else 
		setConfig();
	_cgiPath = _root;
	if(!_pathWithAlias.empty()){
		_cgiPath += _pathWithAlias;
	}
	std::string fullPath = _cgiPath + _cgiName;
	std::ifstream file(fullPath.c_str());
	if(file.is_open()){
		file.close();
		return true;
	}
	else {
		return false;
	}

}

void Methods::setCgiArg(){
	
	size_t trigger = _parsedRequest.uri.find_first_of('?');
	if(trigger != std::string::npos){
		_cgiArg = _parsedRequest.uri.substr(trigger+1);
	}
}


void Methods::cgiHandler(){
	setCgiName();
	if(setCgiPath() == true){
		setCgiArg();
	}
	else{
		fillError("404");
		return;
	}
	if(isMethodAllowed(_methods, _parsedRequest.method) == true){
		if(_parsedRequest.method == "GET"){
			_content = runCgiAndGetOutput( _cgiName.c_str(), _cgiArg, 0, _cgiPath.c_str(), &_ret, _parsedRequest.uri, _fdArray);
		}
		if(_parsedRequest.method == "POST"){			
			_content = runCgiAndGetOutput(_cgiName.c_str(), _parsedRequest.body, 0, _cgiPath.c_str(), &_ret, _parsedRequest.uri, _fdArray);
		}
		setResponse();
	}
	else
		fillError("405");
}


bool Methods::checkPhpCgi() {
	size_t trigger= _parsedRequest.uri.find_first_of('?');
	if(trigger != std::string::npos){
		std::string firstCut = _parsedRequest.uri.substr(0, trigger);
		size_t lastDot = firstCut.find_last_of('.');
		if(lastDot != std::string::npos){
			std::string ext = firstCut.substr(lastDot);
			Log("EXT FOUND :" + ext);
			if(ext == ".php")
				return true;
			else
				return false;

		}
	}
	size_t dot = _parsedRequest.uri.find_first_of('.');
	if(dot != std::string::npos){
		std::string ext = _parsedRequest.uri.substr(dot);
		if(ext == ".php")
			return true;
		else
			return false;
	}
	std::string type = _parsedRequest.headers["Content-Type"];
	if(type == "application/x-www-form-urlencoded")
		return true;
	return false;
}


std::string getCurrentWorkingDirectory() {
	char buffer[2048];
	if (getcwd(buffer, sizeof(buffer)) != NULL)
		return std::string(buffer);
	else
		return std::string();
}


int pipexec(char **arg, char **envp, int *ret, int fdtemp, pid_t &childPid) 
{
	pid_t pid = fork();
	
	if (pid < 0) 
	{
		*ret = -1;
		return 0;
	}
	else if (pid == 0)
	{
		// Child process
		if (dup2(fdtemp, STDOUT_FILENO) < 0)
		{
			perror("dup2 STDOUT");
			exit(EXIT_FAILURE);
		}
		if (dup2(fdtemp, STDERR_FILENO) < 0)
		{
			perror("dup2 STDERR");
			exit(EXIT_FAILURE);
		}
		execve(arg[0], arg, envp);
		perror("execve");
		exit(EXIT_FAILURE);
	}
	else
	{
		// Parent process
		childPid = pid; // set the return parameter childPid to monitor it later
		*ret = 0;       // CGI started, still active
		return 1;
	}
}


bool check_extention_php(const char *scriptname)
{
	std::string str(scriptname);
	if (str[str.size() - 1] == 'p' && str[str.size() - 2] == 'h' && str[str.size() - 3] == 'p')
		return true;
	return false;
}

void Methods::cgi_php_handler(int *ret, const char *scriptname, std::string *querystring, bool reqtype, const char *path, int fdtemp, std::string uri)
{
	std::vector<std::string> 	vec;
	std::ostringstream 			script_filename, request_method, content_type, content_length, redirect_status, query_string, path_info, request_uri;
	const char 					*arg[2];
	std::string commandPath;

	arg[0] = NULL;
	if (check_extention_php(scriptname))
	{
		arg[0] = "/usr/bin/php-cgi";
		script_filename << "SCRIPT_FILENAME=" << path << scriptname;
		vec.push_back(script_filename.str());
	}
	else
	{
		script_filename << getCurrentWorkingDirectory() << "/" << path << scriptname;
		vec.push_back(script_filename.str());
		commandPath = script_filename.str();
		arg[0] = commandPath.c_str();
	}
	arg[1] = NULL;

	// 0 = GET, 1 = POST
	const char *method = (!reqtype) ? "GET" : "POST";
	request_method << "REQUEST_METHOD=" << method;
	vec.push_back(request_method.str());
	if (reqtype)
	{
		content_type << "CONTENT_TYPE=application/x-www-form-urlencoded";
		vec.push_back(content_type.str());
	}

	content_length << "CONTENT_LENGTH=" << (reqtype ? querystring->size() : 0);
	vec.push_back(content_length.str());

	vec.push_back("SERVER_PROTOCOL=HTTP/1.1");

	redirect_status << "REDIRECT_STATUS=200";
	vec.push_back(redirect_status.str());

	path_info << "PATH_INFO=" << uri;
	vec.push_back(path_info.str());

	vec.push_back("SCRIPT_NAME=/ubuntu_cgi_tester");

	request_uri << "REQUEST_URI=" << uri;
	vec.push_back(request_uri.str());

	query_string << "QUERY_STRING=" << *querystring;
	vec.push_back(query_string.str());

	char **envp_arr = new char*[vec.size() + 1];
	for (size_t i = 0; i < vec.size(); ++i)
	{
		envp_arr[i] = new char[vec[i].size() + 1];
		std::strcpy(envp_arr[i], vec[i].c_str());
	}
	envp_arr[vec.size()] = NULL;
	
	pid_t childPid;
	if (!pipexec((char **)arg, envp_arr, ret, fdtemp, childPid))
		std::cout << "!!!!!!!!!!! SOMETHING WENT WRONG WITH PIPEX !!!!!!!!!!!" << std::endl;

	// Here, we stock childPid in Client to check it in poll main loop
	_client->setCgiPid(childPid);
	
	for (size_t i = 0; i < vec.size(); ++i)
	{
		delete[] envp_arr[i];
	}
	delete[] envp_arr;
}


// Function to start handling CGI in a asynchrone way
void Methods::startCgiAsync() {

	int pipefd[2];
	if (pipe(pipefd) < 0) {
		perror("pipe");
		fillError("500");
		return;
	}
	// Start the CGI using pipefd[1] for writing
	cgi_php_handler(&_ret, _cgiName.c_str(), &_cgiArg, reqType, _cgiPath.c_str(), pipefd[1], _parsedRequest.uri);
	close(pipefd[1]);

	// Put the pipe in non-blocking way
	int flags = fcntl(pipefd[0], F_GETFL, 0);
	fcntl(pipefd[0], F_SETFL, flags | O_NONBLOCK);

	// Stock CGI's PID and fd in the client
	_client->setCgiPipe(pipefd[0]);
	
	struct pollfd cgiFd;
	cgiFd.fd = pipefd[0];
	cgiFd.events = POLLIN;
	_fdArray.push_back(cgiFd);
}

