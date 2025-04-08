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
		if(_parsedRequest.method == "GET")
		{
			startCgiAsync(0, _cgiArg);
		}
		if(_parsedRequest.method == "POST")
		{
			startCgiAsync(1, _parsedRequest.body);
		}
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


int pipexec(char **arg, char **envp, int *ret, int stdoutFd, int stdinFd, pid_t &childPid)
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
        if (stdinFd != -1) {
            if (dup2(stdinFd, STDIN_FILENO) < 0) {
                perror("dup2 STDIN");
                exit(EXIT_FAILURE);
            }
            // On peut fermer stdinFd après dup2
            close(stdinFd);
        }
        if (dup2(stdoutFd, STDOUT_FILENO) < 0)
        {
            perror("dup2 STDOUT");
            exit(EXIT_FAILURE);
        }
        if (dup2(stdoutFd, STDERR_FILENO) < 0)
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
        childPid = pid;
        *ret = 0;
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

void Methods::cgi_php_handler(int *ret, const char *scriptname, std::string *querystring, bool reqtype,
                               const char *path, int stdoutFd, int stdinFd, std::string uri)
{
    std::vector<std::string> vec;
    std::ostringstream script_filename, request_method, content_type, content_length,
                         redirect_status, query_string, path_info, request_uri;
    const char *arg[2];
    std::string commandPath;
    arg[0] = NULL;
    if (check_extention_php(scriptname))
    {
        arg[0] = "/usr/bin/php-cgi";
        script_filename << "SCRIPT_FILENAME=" << path << scriptname;
        std::cout << "FILE NAME :" << path << scriptname << std::endl;
        vec.push_back(script_filename.str());
    }
    else
    {
        script_filename << getCurrentWorkingDirectory() << "/" << path << scriptname;
        std::cout << "Script_filename :" << script_filename.str() << std::endl;
        vec.push_back(script_filename.str());
        commandPath = script_filename.str();
        arg[0] = commandPath.c_str();
    }
    arg[1] = NULL;

    // 0 = GET, 1 = POST
    const char *meth = (!reqtype) ? "GET" : "POST";
    request_method << "REQUEST_METHOD=" << meth;
    vec.push_back(request_method.str());
    if (reqtype)
    {
        content_type << "CONTENT_TYPE=application/x-www-form-urlencoded";
        vec.push_back(content_type.str());
    }

    content_length << "CONTENT_LENGTH=" << querystring->size();
    vec.push_back(content_length.str());

    vec.push_back("SERVER_PROTOCOL=HTTP/1.1");

    redirect_status << "REDIRECT_STATUS=200";
    vec.push_back(redirect_status.str());

    path_info << "PATH_INFO=" << uri;
    vec.push_back(path_info.str());

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
    std::cout << "ARG 0 :" << arg[0] << std::endl;

    // On appelle pipexec en transmettant stdoutFd et aussi stdinFd
    pid_t childPid;
    if (!pipexec((char **)arg, envp_arr, ret, stdoutFd, stdinFd, childPid))
        std::cout << "!!!!!!!!!!! SOMETHING WENT WRONG WITH PIPEX !!!!!!!!!!!" << std::endl;

    // Stockage du childPid dans le client pour le suivi via poll
    _client->setCgiPid(childPid);

    for (size_t i = 0; i < vec.size(); ++i)
    {
        delete[] envp_arr[i];
    }
    delete[] envp_arr;
}


// Function to start handling CGI in a asynchrone way
void Methods::startCgiAsync(int reqtype, std::string cgiArg)
{
    int stdoutPipe[2];
    if (pipe(stdoutPipe) < 0) {
        perror("pipe stdout");
        fillError("500");
        return;
    }

    int stdinPipe[2];
    if (reqtype == 1) { // POST
        if (pipe(stdinPipe) < 0) {
            perror("pipe stdin");
            fillError("500");
            return;
        }
    }
    else {
        // Pour GET, on ne redirige pas STDIN, on passera -1
        stdinPipe[0] = -1;
        stdinPipe[1] = -1;
    }

    std::cout << "Pipe created: stdout read fd = " << stdoutPipe[0] << ", write fd = " << stdoutPipe[1] << std::endl;
    if (reqtype == 1) {
        std::cout << "Pipe created: stdin read fd = " << stdinPipe[0] << ", write fd = " << stdinPipe[1] << std::endl;
    }

    // Appel de cgi_php_handler en transmettant stdoutPipe[1] et stdinPipe[0] (ou -1 pour GET)
    cgi_php_handler(&_ret, _cgiName.c_str(), &cgiArg, reqtype, _cgiPath.c_str(),
                    stdoutPipe[1], (reqtype == 1 ? stdinPipe[0] : -1), _parsedRequest.uri);
    close(stdoutPipe[1]); // On ferme le côté écriture de stdout dans le parent

    if (reqtype == 1) {
        // Pour POST, écrire le contenu de cgiArg dans le pipe stdin
        ssize_t totalWritten = 0, written = 0;
        const char* inputData = cgiArg.c_str();
        size_t inputLength = cgiArg.size();
        while (totalWritten < inputLength) {
            written = write(stdinPipe[1], inputData + totalWritten, inputLength - totalWritten);
            if (written < 0) {
                perror("write to stdinPipe");
                break;
            }
            totalWritten += written;
        }
        close(stdinPipe[1]); // Fermer le côté écriture du pipe STDIN dans le parent
    }

    _client->setRet(200);

    // Mettre stdoutPipe[0] en mode non bloquant
    int flags = fcntl(stdoutPipe[0], F_GETFL, 0);
    fcntl(stdoutPipe[0], F_SETFL, flags | O_NONBLOCK);

    // Stocker le fd du pipe CGI (stdoutPipe[0]) dans le client
    _client->setCgiPipe(stdoutPipe[0]);

    // Ajouter le fd stdout du CGI à votre tableau de poll
    struct pollfd cgiFd;
    cgiFd.fd = stdoutPipe[0];
    cgiFd.events = POLLIN;
    _fdArray.push_back(cgiFd);

    std::cout << "PIPE FD in startCgiAsync = " << stdoutPipe[0] << std::endl;
}
