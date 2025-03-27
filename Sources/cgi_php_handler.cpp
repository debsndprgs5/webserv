

// Using ternaries to prevent making 2 functions for almost the same result, supporting
// either POST and GET for php
// 0 = GET 1 = POST

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include "../Includes/Methods.hpp"

int pipexec(char **arg, char **envp, int *ret, int fdtemp) 
{
	pid_t pid = fork();
	
	if (pid < 0) 
	{
		*ret = -1;
		return 0;
	}
	else if (pid == 0)
	{
		// Child Process
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
		// Parent Process
		int status;
		if (waitpid(pid, &status, 0) < 0)
		{
			*ret = -1;
			return 0;
		}
		else
		{
			if (WIFEXITED(status))
				*ret = WEXITSTATUS(status);
			else
				*ret = -1;
		}
		return 1;
	}
}

void cgi_php_handler(int *ret, const char *scriptname, std::string *querystring, bool reqtype, const char *path, int fdtemp)
{
	const char *arg[] = { "/usr/bin/php-cgi", NULL };
	std::vector<std::string> vec;
	std::ostringstream script_name, request_method, content_type, content_length, query_string;
	
	script_name << "SCRIPT_FILENAME=" << path << "/" << scriptname;
	vec.push_back(script_name.str());

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

	query_string << "QUERY_STRING=" << *querystring;
	vec.push_back(query_string.str());

	char **envp_arr = new char*[vec.size() + 1];
	for (size_t i = 0; i < vec.size(); ++i)
	{
		envp_arr[i] = new char[vec[i].size() + 1];
		std::strcpy(envp_arr[i], vec[i].c_str());
	}
	envp_arr[vec.size()] = NULL;

	if (!pipexec((char **)arg, envp_arr, ret, fdtemp))
		std::cout << "!!!!!!!!!!! SOMETHING WENT WRONG WITH PIPEX !!!!!!!!!!!" << std::endl;

	for (size_t i = 0; i < vec.size(); ++i)
	{
		delete[] envp_arr[i];
	}
	delete[] envp_arr;
}

// Use a pipe to start the CGI and get output as a string
std::string runCgiAndGetOutput(const char *scriptname, std::string &queryString, bool reqType, const char *path, int *ret)
{
	// Create a pipe : pipefd[0] read, pipefd[1] write
	int pipefd[2];
	if (pipe(pipefd) < 0)
	{
		perror("pipe");
		return "";
	}

	//Call CGI function with write pipe
	cgi_php_handler(ret, scriptname, &queryString, reqType, path, pipefd[1]);
	
	close(pipefd[1]);

	//Read from the pipe
	std::string output;
	char buffer[1024];
	ssize_t bytesRead;
	while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0)
	{
		buffer[bytesRead] = '\0';
		output.append(buffer);
	}
	close(pipefd[0]);
	
	return output;
}