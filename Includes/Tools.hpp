#pragma once
#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <cstdio>
#include <vector>


void ExitWithMessage(int i, std::string str);
int ReturnWithMessage(int i, std::string str);
void Log(std::string str);