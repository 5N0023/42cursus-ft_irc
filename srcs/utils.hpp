#pragma once

#include "server.hpp"
class user;
size_t ToSize_T(std::string &);
bool has_char(std::string);
user getUser_str(std::string,std::vector<user>);
std::string getreason(std::vector<std::string>,size_t);
std::vector<std::string> split(const std::string& input, char delimiter);
std::vector<std::string> splitCommand(std::string command);
std::map<std::string, std::string> parseChannels(std::string channels,std::string keys);
std::string getLocalIP();