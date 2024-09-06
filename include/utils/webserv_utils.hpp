#ifndef __WEBSERV_UTILS_HPP__
#define __WEBSERV_UTILS_HPP__

#include "../servers/Server.hpp"
#include "utils.hpp"
#include <string>
#include <map>

class Server;

namespace webserv_utils
{

std::string getServerBlock(std::ifstream &);
void ft_error(int, std::string);
void displayServers(std::vector<Server *> &);
void parseUrl(std::string, std::vector<std::string> &, std::vector<std::string> &);
bool methodIsImplemented(const std::string &method);
bool httpVersionIsSupported(const std::string &version);
bool socketIsSet(std::map<int, struct sockaddr_in> &, struct sockaddr_in &);
void printSocketAddress(struct sockaddr_in &_socketAddr);



}; // namespace webserv_utils

#endif
