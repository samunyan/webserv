#include "../include/servers/Webserv.hpp"

static bool validFile(const std::string &);

int main(int argc, char **argv)
{
	if (argc > 2)
	{
		std::cerr << "ERROR\nExpected : ./webserv [configuration file path]" << std::endl;
		return EXIT_FAILURE;
	}

	std::string conf_file;
	(argc == 2) ? (conf_file = argv[1]) : (conf_file = "./conf/default.conf");
	if (!validFile(conf_file))
	{
		std::cerr << "ERROR\nInvalid configuration file" << std::endl;
		return EXIT_FAILURE;
	}

	Webserv webserver(conf_file);

	try
	{
		webserver.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

static bool validFile(const std::string &file)
{
	size_t pos = file.find_last_of('.');
	std::string extension = &file[pos];

	if (!pos || (extension != ".conf" && extension != ".cnf"))
		return false;

	std::ifstream ifs;

	ifs.open(file.c_str());
	if (ifs.fail())
		return false;
	ifs.close();
	return true;
}