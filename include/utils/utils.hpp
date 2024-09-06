#ifndef __UTILS_HPP__
# define __UTILS_HPP__

#include "../servers/Server.hpp"
#include <algorithm>
#include <arpa/inet.h>
#include <csignal>
#include <ctime>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <map>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sstream>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#define DISPLAY_SERVERS false
#define DISPLAY_REQUEST false
#define DISPLAY_SERVER_FOR_REQUEST false
#define DISPLAY_LOCATION_FOR_REQUEST false
#define DISPLAY_RESPONSE false

#define DIGITS "0123456789"
#define BUFFER_SIZE 10000
#define MAX_LISTEN 1000

std::string ft_pop_back(const std::string &);
int ft_stoi(const std::string &);
std::string ft_to_string(int);
std::string trim(const std::string &);
void log(const std::string &line, int client_fd, const std::string &url, int type);

#endif