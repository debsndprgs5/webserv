#pragma once
#include "ParsingDataStructs.hpp"

HttpConfig parseHttp(std::ifstream &in);
LocationConfig parseLocation(std::ifstream &in, const std::string &firstLine);
ServerConfig parseServer(std::ifstream &in);
std::string trim(const std::string &s);
std::vector<std::string> split(const std::string &s);
bool parseBool(const std::string &s);
int parsePort(const std::string &s);
int Parse(char *file);
std::string parseIP(const std::string &s);

