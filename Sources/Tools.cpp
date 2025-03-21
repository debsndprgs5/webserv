#include "../Includes/Tools.hpp"

void ExitWithMessage(int i, std::string str){

	Log("Critical Error : " + str);
	exit(i);
}

int ReturnWithMessage(int i, std::string str){
	if(i != 0)
		Log("Error : " + str);
	else 
		Log(str);
	return i;
}

void Log(std::string str){
	std::cout << str << std::endl;
}

