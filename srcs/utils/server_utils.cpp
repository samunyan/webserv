#include "../../include/utils/server_utils.hpp"

namespace server_utils
{

std::string getOptionName(const std::string &str)
{
	std::string result = str.substr(0, str.find_first_of(" \t"));

	return result;
}

std::string getOptionValue(const std::string &str)
{
	std::string result = &str[str.find_first_of(" \t")];

	result = &result[result.find_first_not_of(" \t")];

	while (result[result.size() - 1] == '{' || result[result.size() - 1] == ';' || result[result.size() - 1] == ' ' ||
	       result[result.size() - 1] == '\t')
		result = ft_pop_back(result);

	return result;
}

std::string getLocationBlock(std::stringstream &ifs)
{
	std::string location_block, buffer;

	while (location_block[location_block.size() - 1] != '}')
	{
		getline(ifs, buffer);
		if (buffer[0] == '{')
			continue;
		location_block.append(buffer);
		if (location_block[location_block.size() - 1] != '}')
			location_block.append("\n");
	}
	location_block = ft_pop_back(location_block);
	if (location_block[location_block.size() - 1] == '\n')
		location_block = ft_pop_back(location_block);
	return location_block;
}

t_location newLocation(const std::string &location_name, const std::string &location_block, std::string &root, std::string autoindex)
{
	t_location loc;
	int option, pos;
	std::stringstream ifs(location_block);
	std::string method, buffer, name, value, slash = "/",
	                                         option_list[4] = {"root", "index", "allow_methods", "autoindex"};

	bool allow_methods_is_not_defined = true;
	loc.location = location_name;
	loc.root = "";
	loc.index = "";
	loc.autoindex = autoindex;
	loc.valid = false;

	while (!ifs.eof())
	{
		buffer.clear();
		getline(ifs, buffer);
		if (buffer[buffer.size() - 1] != ';')
		{
			ft_error(1, buffer, buffer);
			return loc;
		}
		name = getOptionName(buffer);
		value = getOptionValue(buffer);
		for (int i = 0; i < 4; i++)
		{
			option = i;
			if (name == option_list[i])
				break;
		}
		switch (option)
		{
		case 0:
			if (loc.root != "")
			{
				ft_error(0, value, "loc.root");
				return loc;
			}
			loc.root = value;
			break;
		case 1:
			if (loc.index != "")
			{
				ft_error(0, value, "loc.index");
				return loc;
			}
			loc.index = value;
			break;
		case 2:
			allow_methods_is_not_defined = false;
			value.push_back(' ');
			while (1)
			{
				pos = value.find_first_of(" \t");
				method = value.substr(0, pos);
				if (!method.empty() && method != "GET" && method != "DELETE" && method != "POST" && method != "HEAD" && method != "PUT" &&
				    method != "CONNECT" && method != "OPTIONS" && method != "TRACE" && method != "PATCH")
				{
					ft_error(3, method, "");
					return loc;
				}
				if (!loc.methods.empty())
				{
					for (std::vector<std::string>::iterator it = loc.methods.begin(); it != loc.methods.end(); it++)
					{
						if (*it == method)
						{
							ft_error(0, value, "loc.allow_method");
							return loc;
						}
					}
				}
				if (!method.empty())
					loc.methods.push_back(method);
				value = &value[pos + 1];
				if (!value.size())
					break;
				value = &value[value.find_first_not_of(" \t")];
			}
			break;
		case 3:
			if (value != "on" && value != "off")
			{
				ft_error(5, value, "");
				return loc;
			}
			loc.autoindex = value;
			break;
		default:
			ft_error(4, name, "");
			return loc;
		}
	}
	if (loc.root == "")
		loc.root = root;
	if (loc.methods.empty() && allow_methods_is_not_defined)
		loc.methods = Webserv::implementedMethods;
	loc.valid = true;
	return loc;
}

void ft_error(int type, std::string value, std::string option)
{
	std::cerr << "ERROR\n";
	switch (type)
	{
	case 0:
		std::cerr << option << " " << value << ": duplicate" << std::endl;
		break;
	case 1:
		std::cerr << option << " " << value << ": missing ';'" << std::endl;
		break;
	case 2:
		std::cerr << option << " " << value << ": Not a number" << std::endl;
		break;
	case 3:
		std::cerr << value << ": invalid method" << std::endl;
		break;
	case 4:
		std::cerr << value << ": invalid option" << std::endl;
		break;
	case 5:
		std::cerr << "autoindex " << value << ": invalid value" << std::endl;
		break;
	case 6:
		std::cerr << "Listen directive is empty" << std::endl;
		break;
	case 7:
		std::cerr << "listen: invalid value" << std::endl;
		break;
	case 8:
		std::cerr << option << " " << value << ": missing url" << std::endl;
		break;
	case 9:
		std::cerr << value << ": no such directory" << std::endl;
		break;
	}
}

bool setSocketAddress(const std::string &ip_address, const std::string &port_num, struct sockaddr_in *socket_addr)
{
	struct addrinfo hints = {};
	struct addrinfo *res = NULL;
	int status;

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	status = getaddrinfo(ip_address.c_str(), port_num.c_str(), &hints, &res);
	if (status != 0 || !res)
		return false;
	*socket_addr = *(struct sockaddr_in *)res->ai_addr;
	freeaddrinfo(res);
	return true;
}

int getSocketAddress(int socket, struct sockaddr_in *addr)
{
	socklen_t len = sizeof *addr;
	return (getsockname(socket, (struct sockaddr *)addr, &len));
}

}; // namespace server_utils
