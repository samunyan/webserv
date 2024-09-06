#include "../../include/utils/webserv_utils.hpp"

namespace webserv_utils
{

std::string getServerBlock(std::ifstream &ifs)
{
	int brackets = 1;
	std::string buffer, server_block = "";

	while (brackets)
	{
		getline(ifs, buffer);
		buffer = trim(buffer);
		server_block.append(buffer);
		server_block.append("\n");
		if (buffer.find('{') != buffer.npos)
			brackets++;
		else if (buffer.find('}') != buffer.npos)
			brackets--;
	}
	server_block = ft_pop_back(server_block);
	server_block = ft_pop_back(server_block);
	while (server_block[server_block.size() - 1] == ' ' || server_block[server_block.size() - 1] == '\t' ||
	       server_block[server_block.size() - 1] == '\n')
		server_block = ft_pop_back(server_block);
	return server_block;
}

void ft_error(int type, std::string value)
{
	(void)value;
	switch (type)
	{
	case 0:
		std::cerr << "Select error" << std::endl;
		break;
	case 1:
		std::cerr << "Server failed to accept incoming connection from ADDRESS: ";
		break;
	}
}

bool socketIsSet(std::map<int, struct sockaddr_in> &socket_list, struct sockaddr_in &addr)
{
	for (std::map<int, struct sockaddr_in>::iterator it = socket_list.begin(); it != socket_list.end(); it++)
	{
		if (it->second.sin_port == addr.sin_port && it->second.sin_addr.s_addr == addr.sin_addr.s_addr)
			return true;
	}
	return false;
}

void printSocketAddress(struct sockaddr_in &_socketAddr)
{
	char s[INET_ADDRSTRLEN] = {};

	inet_ntop(AF_INET, (void *)&_socketAddr.sin_addr, s, INET_ADDRSTRLEN);
	std::cout << s << ":" << ntohs(_socketAddr.sin_port);
}

void displayServers(std::vector<Server *> &server_list)
{
	std::string value;
	int iValue;
	std::vector<int> port_list;
	std::map<std::string, t_location> location_list;
	std::vector<std::string> method_list;

	std::cout << std::endl;

	for (std::vector<Server *>::iterator it = server_list.begin(); it != server_list.end(); it++)
	{
		static int i;
		std::cout << "*******   "
		          << "server #" << i++ << "   *******" << std::endl;
		value = (*it)->getIndex();
		std::cout << "Index : " << value << std::endl;
		value = (*it)->getRoot();
		std::cout << "Root : " << value << std::endl;
		std::vector<std::string>::const_iterator cit;
		std::cout << "Server name : " << std::endl;
		for (cit = (*it)->getServerNames().begin(); cit != (*it)->getServerNames().end(); ++cit)
			std::cout << "  - " << *cit << std::endl;
		iValue = (*it)->getBodySize();
		std::cout << "Client max body size : " << iValue << std::endl;
		std::cout << "Locations :\n";
		location_list = (*it)->getLocationlist();
		if (!location_list.empty())
		{
			for (std::map<std::string, t_location>::iterator it = location_list.begin(); it != location_list.end();
			     it++)
			{
				std::cout << "  - " << it->second.location << " :\n";
				value = it->second.root;
				std::cout << "    - Root : " << value << std::endl;
				value = it->second.index;
				std::cout << "    - Index : " << value << std::endl;
				std::cout << "    - Autoindex : " << it->second.autoindex << std::endl;
				method_list = it->second.methods;
				std::cout << "    - Allowed methods :\n";
				for (std::vector<std::string>::iterator it = method_list.begin(); it != method_list.end(); it++)
						std::cout << "       - " << *it << std::endl;
			}
		}
		std::cout << "Listening on :\n";
		std::vector<struct sockaddr_in> endpoints = (*it)->getEndPoints();
		for (std::vector<struct sockaddr_in>::iterator it = endpoints.begin(); it != endpoints.end(); ++it)
		{
			std::cout << "    - ";
			printSocketAddress(*it);
			std::cout << std::endl;
		}
		std::cout << std::endl;
		std::cout << "*****************************" << std::endl;
	}
}

void parseUrl(std::string folder, std::vector<std::string> &url_list, std::vector<std::string> &folder_list)
{
	DIR *dir = opendir(folder.c_str());
	struct dirent *file;
	std::string file_name, extension, sub_folder, folder_cpy = folder;
	size_t pos;

	file = readdir(dir);

	while (file)
	{
		file_name = file->d_name;
		if (file_name == "." || file_name == "..")
		{
			file = readdir(dir);
			continue;
		}
		pos = file_name.find_last_of(".");
		if (pos != std::string::npos)
			extension = &file_name[pos];
		else
			extension.clear();
		if (extension == "")
		{
			sub_folder = folder_cpy.append(file_name);
			folder_cpy = folder;
			sub_folder.append("/");
			folder_list.push_back(&sub_folder[1]);
			parseUrl(sub_folder, url_list, folder_list);
		}
		else
		{
			file_name = folder_cpy.append(file_name);
			folder_cpy = folder;
			url_list.push_back(file_name);
		}

		file = readdir(dir);
	}
	closedir(dir);
}
bool methodIsImplemented(const std::string &method)
{
	return (method == "GET" || method == "POST" || method == "DELETE");
}

bool httpVersionIsSupported(const std::string &version)
{
	return (version == "HTTP/1.1");
}
}; // namespace webserv_utils
