#include "../../include/utils/utils.hpp"

std::string ft_pop_back(const std::string &str)
{
	std::string result;
	size_t pos = str.size() - 1;

	for (size_t i = 0; i < pos; i++)
		result += str[i];

	return result;
}

std::string ft_to_string(int nb)
{
	std::string result = "", ch;

	while (nb > 0)
	{
		ch = nb % 10 + '0';
		ch += result;
		result = ch;
		nb /= 10;
	}
	return ch;
}

std::string trim(const std::string &str)
{
	std::string result;

	if (!str.empty() && str.find_first_not_of(" \t") != std::string::npos)
	{
		result = &str[str.find_first_not_of(" \t")];
		while (result[result.size() - 1] == ' ' || result[result.size() - 1] == '\t')
			result = ft_pop_back(result);
	}
	return result;
}

int ft_stoi(const std::string &str)
{
	int result = 0;
	size_t size = str.size();

	for (size_t i = 0; i < size; i++)
		result = result * 10 + str[i] - '0';

	return result;
}

void log(const std::string &line, int client_fd, const std::string &url, int type)
{
	time_t tm = std::time(NULL);
	char *dt = ctime(&tm);
	std::string odt = ft_pop_back(dt);
	std::ofstream log_file;

	log_file.open("./webserv.log", std::ofstream::app);

	log_file << odt << " - ";

	switch (type)
	{
	case 0:
		log_file << line << "\n";
		break;
	case 1:
		log_file << client_fd << ": " << line << "\n";
		break;
	case 2:
		log_file << ": Request ( " << url << " ) received from " << client_fd << "\n";
		break;
	case 3:
		log_file << ": Reponse ( " << url << " ) sent to " << client_fd << "\n";
		break;
	}

	log_file.close();
}