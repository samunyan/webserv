#include "../../include/servers/Webserv.hpp"

using namespace webserv_utils;

static std::vector<std::string> getImplementedMethods()
{
	std::vector<std::string> v;
	v.push_back("GET");
	v.push_back("POST");
	v.push_back("DELETE");
	return v;
}

std::vector<std::string> Webserv::implementedMethods = getImplementedMethods();

Webserv::Webserv()
{
}

Webserv::Webserv(const std::string &conf_file) : _conf(conf_file)
{
}

Webserv::~Webserv()
{
	for (std::vector<Server *>::const_iterator it = _server_list.begin(); it != _server_list.end(); it++)
		delete (*it);
	for (std::vector<int>::const_iterator it = _listen_socket_list.begin(); it != _listen_socket_list.end(); it++)
		close(*it);
	for (std::vector<Request *>::const_iterator it = _request_list.begin(); it != _request_list.end(); it++)
	{
		close((*it)->getSocket());
		delete (*it);
	}
}

void Webserv::run()
{
	parseConf();
	startServer();
	startListen();
}


void Webserv::parseConf()
{
	std::ifstream infile(_conf.c_str());
	std::string buffer, server_block;
	Server *server;

	while (!infile.eof())
	{
		getline(infile, buffer);
		if (buffer == "server {")
		{
			server_block = getServerBlock(infile);
			server = new Server;
			if (!server->parseServer(server_block))
			{
				delete server;
				throw confFailureException();
			}
			_server_list.push_back(server);
		}
	}
	infile.close();

	if (DISPLAY_SERVERS)
		displayServers(_server_list);
}

void Webserv::startServer()
{
	std::vector<struct sockaddr_in> address_list;
	struct sockaddr_in addr;
	int listen_socket;
	int reuse = true;

	for (std::vector<Server *>::iterator server_it = _server_list.begin(); server_it != _server_list.end(); server_it++)
	{
		address_list = (*server_it)->getEndPoints();
		for (std::vector<struct sockaddr_in>::iterator addr_it = address_list.begin(); addr_it != address_list.end();
		     addr_it++)
		{
			if (!socketIsSet(_socket_list, *addr_it))
			{
				listen_socket = socket(AF_INET, SOCK_STREAM, 0);
				if (listen_socket < 0)
					throw openSocketException();

				if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
					throw setSocketoptionException();

				fcntl(listen_socket, F_SETFL, O_NONBLOCK);
				_listen_socket_list.push_back(listen_socket);
				_global_socket_list.push_back(listen_socket);

				addr = *addr_it;
				if (bind(listen_socket, (sockaddr *)&addr, sizeof(addr)) < 0)
					throw bindException();

				if (listen(listen_socket, MAX_LISTEN) < 0)
					throw listenException();
				_socket_list[listen_socket] = *addr_it;
			}
		}
	}
}

bool run_webserv = true;

static void sigHandler(int sig_num)
{
	if (sig_num == SIGINT)
		run_webserv = false;
}

void Webserv::startListen()
{
	std::cout << "\n*** Webserv started ***\n" << std::endl;
	log("Webserv started", -1, "", 0);

	int select_return, max, step = 1;
	fd_set readfds, writefds;

	FD_ZERO(&writefds);
	FD_ZERO(&readfds);

	signal(SIGINT, sigHandler);

	while (run_webserv)
	{
		errno = 0;
		for (std::vector<int>::const_iterator it = _global_socket_list.begin(); it != _global_socket_list.end(); it++)
			FD_SET(*it, &readfds);
		max = *std::max_element(_global_socket_list.begin(), _global_socket_list.end()) + 1;
		select_return = select(max, &readfds, &writefds, nullptr, nullptr);
		if (errno == EINTR)
			break ;
		if (select_return < 0)
			throw std::runtime_error("Select error : " + std::string(strerror(errno)));
		if (select_return == 0)
			continue;
		if (step == 1)
		{
			step = 2;
			if (!acceptNewConnections(readfds))
			{
				readRequests(readfds, writefds);
				step = 3;
			}
		}
		else if (step == 2)
		{
			readRequests(readfds, writefds);
			step = 3;
		}
		else if (step == 3)
		{
			sendResponses(writefds, readfds);
			step = 1;
		}
	}
	std::cout << "\n*** Webserv stopped ***\n" << std::endl;
	log("Webserv stopped", -1, "", 0);
}

bool Webserv::acceptNewConnections(fd_set &readfds)
{
	int new_socket, opt = 1;
	Request *new_request;
	struct sockaddr_in addr;
	socklen_t addr_len = sizeof(addr);
	bool new_connection = false;

	for (std::vector<int>::iterator it = _listen_socket_list.begin(); it != _listen_socket_list.end(); it++)
	{
		if (FD_ISSET(*it, &readfds))
		{
			FD_CLR(*it, &readfds);
			while (true)
			{
				new_socket = accept(*it, (sockaddr *)&addr, &addr_len);
				if (new_socket < 0)
					break;
				setsockopt(new_socket, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt));
				new_request = new Request(new_socket);
				new_request->getPotentialServers(_server_list, _socket_list[*it]);
				_request_list.push_back(new_request);
				_global_socket_list.push_back(new_socket);
				new_connection = true;
			}
		}
	}
	return new_connection;
}

void Webserv::readRequests(fd_set &readfds, fd_set &writefds)
{
	int socket;
	bool connection;

	std::vector<Request *>::iterator it;
	for (it = _request_list.begin(); it != _request_list.end();)
	{
		socket = (*it)->getSocket();
		if (FD_ISSET(socket, &readfds))
		{
			FD_CLR(socket, &readfds);
			try
			{
				connection = (*it)->retrieveRequest();
				if (!connection)
				{
					close(socket);
					_global_socket_list.erase(find(_global_socket_list.begin(), _global_socket_list.end(), socket));
					delete (*it);
					it = _request_list.erase(it);
					log("connection closed by client", socket, "", 1);
				}
				else
				{
					FD_SET(socket, &writefds);
					it++;
				}
			}
			catch (const std::exception &e)
			{
				close(socket);
				_global_socket_list.erase(find(_global_socket_list.begin(), _global_socket_list.end(), socket));
				delete (*it);
				it = _request_list.erase(it);
				log(e.what(), socket, "", 1);
			}
		}
		else
			it++;
	}
}

void Webserv::sendResponses(fd_set &writefds, fd_set &readfds)
{
	int socket;
	
	std::vector<Request *>::iterator it;
	for (it = _request_list.begin(); it != _request_list.end();)
	{
		socket = (*it)->getSocket();
		if (FD_ISSET(socket, &writefds) && !FD_ISSET(socket, &readfds))
		{
			FD_CLR(socket, &writefds);
			try
			{
				(*it)->_parseRequest();

				if (DISPLAY_REQUEST)
				{
					std::cout << "******* Request on socket " << socket << " (Parsed) *******" << std::endl;
					std::cout << *(*it);
				}

				(*it)->_response->handleRequest();
				(*it)->_response->buildMessage();
				(*it)->_response->sendMessage();
				// TODO what with chunked requests?
				log("", socket, (*it)->_response->getResourcePath(), 3);
				if ((*it)->_headers.count("Connection") &&
				    (std::find((*it)->_headers["Connection"].begin(), (*it)->_headers["Connection"].end(), "keep-alive") ==
				     (*it)->_headers["Connection"].end()))
				{
					close(socket);
					_global_socket_list.erase(find(_global_socket_list.begin(), _global_socket_list.end(), socket));
					log("do close connection", socket, "", 1);
					delete (*it);
					it = _request_list.erase(it);
				}
				else
				{
					*(*it) = Request(socket, (*it)->_potential_servers);
					it++;
				}
			}
			catch (std::exception &e)
			{
				close(socket);
				_global_socket_list.erase(find(_global_socket_list.begin(), _global_socket_list.end(), socket));
				delete (*it);
				it = _request_list.erase(it);
				log(e.what(), socket, "", 1);
			}
		}
		else
			it++;
	}
}
