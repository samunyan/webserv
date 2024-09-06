#ifndef __SERVER_UTILS_HPP__
#define __SERVER_UTILS_HPP__

#include "utils.hpp"
#include <string>
#include <vector>

struct t_location
{
	std::string location;
	std::string root;
	std::string index;
	std::vector<std::string> methods;
	std::string autoindex;
	bool valid;
};

namespace server_utils
{

std::string getOptionName(const std::string &);
std::string getOptionValue(const std::string &);
std::string getLocationBlock(std::stringstream &);
t_location newLocation(const std::string &, const std::string &, std::string &, std::string);
void ft_error(int, std::string, std::string);
bool setSocketAddress(const std::string &ip_address, const std::string &port_num, struct sockaddr_in *socket_addr);
int getSocketAddress(int socket, struct sockaddr_in *addr);

}; // namespace server_utils

#endif
