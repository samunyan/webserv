#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "../servers/Webserv.hpp"
#include "../utils/utils.hpp"
#include "../utils/webserv_utils.hpp"
#include <iostream>
#include <map>
#include <utility>
#include <vector>

#include "../servers/Server.hpp"
#include "Response.hpp"

class Response;
class Server;

class Request
{
  public:
	explicit Request(int socket);
	Request(int socket, const std::vector<Server *> &potential_servers);
	Request(const Request &src);
	Request &operator=(const Request &rhs);
	virtual ~Request();
	bool operator==(const Request &rhs) const;

	bool retrieveRequest();
	void getPotentialServers(std::vector<Server *> &, struct sockaddr_in &);

	int getSocket() const;
	const std::string &getMethod() const;
	const std::string &getRequestTarget() const;
	const std::string &getHTTPVersion() const;
	const std::map<std::string, std::vector<std::string> >&getHeaders() const;
	bool getValueOfHeader(const std::string &key, std::vector<std::string> &value) const;
	const std::string &getBody() const;
	const std::string &getRequest() const;
	bool isChunkedRequest() const;
	size_t getContentLength() const;
	int getErrorStatus() const;
	class readRequestException : public std::exception
	{
	  public:
		virtual const char *what() const throw()
		{
			return "Error while reading request. Client dismissed.";
		}
	};

	friend class Webserv;
	friend class Server;
	friend class Response;

  private:
	Request();
	int _socket;
	std::string _request;
	std::string _method;
	std::string _request_target;
	std::string _http_version;
	std::map<std::string, std::vector<std::string > > _headers;
	std::string _body;
	int _error_status;
	bool _chunked_request;
	size_t _content_length;
	std::vector<Server *> _potential_servers;
	Server *_server;
	std::string _server_location;
	Response *_response;

	void _parseRequest();
	void _parseRequestLine(const std::string &line);
	void _parseHeader(const std::string &line);
	void _retrieveBodyInfo();
	void _parseBody(std::stringstream &ss);
	void _validateParsedRequestLine();
	void _validateParsedHeaders();
	void _validateParsedBody();
	void _validateMethod();

	void _setServer();
	void _setLocation();
};

std::ostream &operator<<(std::ostream &o, const Request &rhs);

#endif
