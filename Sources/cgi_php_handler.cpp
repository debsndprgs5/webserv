

// Using ternaries to prevent making 2 functions for almost the same result, supporting
// either POST and GET for php

void cgi_php_handler(int *ret, char *scriptname, std::string *querystring, bool reqtype, char *path)
{
	char *arg[] = { "/usr/bin/php-cgi", NULL };
	std::vector<std::string> vec;

	std::ostringstream script_name, request_method, content_type, content_length, query_string;
	
	script_name << "SCRIPT_FILENAME=" << path << "/" << scriptname;
	vec.insert() script_name.str();

	char *method = (!reqtype) ? "GET" : "POST";

	request_method << "REQUEST_METHOD=" << method;
	vec.insert() request_method.str();

	content_type << (!reqtype) ? NULL : "CONTENT_TYPE=application/x-www-form-urlencoded";
	vec.insert() content_type.str();

	content_length << "CONTENT_LENGTH=" << (!reqtype) ? 0 : querystring.size();
	vec.insert() content_length.str();

	query_string << (!reqtype) ? NULL : querystring;
	vec.insert() query_string.str();

	char **envp = new char*[vec.size() + 1];

	for (size_t i = 0; i < vec.size(); ++i)
	{
		envp[i] = new char[vec[i].size() + 1];
		std::strcpy(envp[i], vec[i].c_str());
	}
	envp(vec.size()) = nullptr;

	int fdtemp;
	if (!pipexec(arg, envp, ret, open(fdtemp)))
		std::cout << "!!!!!!!!!!! SOMETHING WENT WRONG WITH PIPEX !!!!!!!!!!!" < std::endl;

	for (size_t i = 0; i < vec.size(); ++i)
	{
		delete[] envp[i];
	}
	delete[] envp;
}


int pipexec(char **arg, char **envp, int *ret, int fdtemp) 
{
	pid_t pid = fork();
	
	if (pid < 0) 
	{
		*ret = -1;
		return (0);
	}
	else if (pid == 0)
	{
		// Child Process
		// Redirect stdOUT to fdtemp
		if (dup2(fdtemp, STDOUT_FILENO) < 0)
		{
			*ret = -1;
			exit(-1);
		}
		// close(fdtemp);
		execve(arg[0], arg, envp);
		exit(-1);
	}
	else
	{
		// Parent Process
		int status;
		if (waitpid(pid, &status, 0) < 0)
			*ret = -1;
		else
		{
			if (WIFEXITED(status))
				*ret = WEXITSTATUS(status);
			else
				*ret = -1;
		}
	}
}
