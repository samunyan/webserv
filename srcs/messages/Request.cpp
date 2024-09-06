#include "../../include/messages/Request.hpp"

Request::Request(int socket)
    : _socket(socket), _error_status(0), _chunked_request(false), _content_length(0), _server(nullptr),
      _response(nullptr)
{
	_response = new Response(this);
}

Request::Request(int socket, const std::vector<Server *> &potential_servers)
    : _socket(socket), _error_status(0), _chunked_request(false), _content_length(0),
      _potential_servers(potential_servers), _server(nullptr), _response(nullptr)
{
}

Request::Request(const Request &src)
    : _socket(src._socket), _request(src._request), _method(src._method), _request_target(src._request_target),
      _http_version(src._http_version), _headers(src._headers), _body(src._body), _error_status(src._error_status),
      _chunked_request(src._chunked_request), _content_length(src._content_length),
      _potential_servers(src._potential_servers), _server(src._server), _server_location(src._server_location),
      _response(src._response)
{
}

Request &Request::operator=(const Request &rhs)
{
	if (this == &rhs)
		return *this;
	_request = rhs._request;
	_socket = rhs._socket;
	_method = rhs._method;
	_http_version = rhs._http_version;
	_headers = rhs._headers;
	_body = rhs._body;
	_error_status = rhs._error_status;
	_chunked_request = rhs._chunked_request;
	_content_length = rhs._content_length;
	_potential_servers = rhs._potential_servers;
	_server = rhs._server;
	_server_location = rhs._server_location;
	delete this->_response;
	_response = new Response(this);
	return *this;
}

Request::~Request()
{
	delete _response;
}

bool Request::operator==(const Request &rhs) const
{
	return this->_socket == rhs._socket;
}

void Request::_parseRequestLine(const std::string &line)
{
	std::stringstream ss(line);
	ss >> _method >> _request_target >> _http_version;
}

void Request::_parseHeader(const std::string &line)
{
	size_t pos = line.find(':');
	std::string key = line.substr(0, pos);
	std::string tmp = line.substr(pos + 1);
	std::vector<std::string> value;

	std::istringstream iss(tmp);
	while (getline(iss, tmp, ','))
	{
		tmp = trim(tmp);
		value.push_back(tmp);
	}
	_headers.insert(std::make_pair(key, value));
}

void Request::_retrieveBodyInfo()
{
	if (_headers.count("Content-Length") == 1)
	{
		if (_chunked_request)
			_error_status = 400;
		std::stringstream ss;
		ss << _headers["Content-Length"][0];
		ss >> _content_length;
	}
	if (_headers.count("Transfer-Encoding") == 1)
	{
		_chunked_request = true;
	}
}


void Request::_parseBody(std::stringstream &ss)
{
	size_t size = ss.str().size() - ss.tellg();
	if (!isChunkedRequest())
	{
		char c;
		for (size_t i(0); i < size; ++i)
		{
			ss.read(&c, sizeof(c));
			_body += c;
		}
	}
	else
		; // TODO parse function for chunked body
}

void Request::_parseRequest()
{
	std::stringstream ss;
	ss.write(_request.c_str(), _request.size());

	std::string line;
	getline(ss, line);
	line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
	_parseRequestLine(line);

	while (!_error_status)
	{
		getline(ss, line);
		line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
		if (line.find(':') != std::string::npos)
			_parseHeader(line);
		else if (line.empty())
		{
			_validateParsedRequestLine();
			_validateParsedHeaders();
			_setServer();
			_setLocation();
			_validateMethod();
			if (!_error_status)
			{
				_retrieveBodyInfo();
				if (_error_status)
					return;
				_parseBody(ss);
				_validateParsedBody();
			}
			return;
		}
		else
			_error_status = 400;
	}
}

void Request::_validateParsedRequestLine()
{
	if (_method.empty() || _request_target.empty() || _http_version.empty())
		_error_status = 400;
	else if (!webserv_utils::methodIsImplemented(_method))
		_error_status = 501;
	else if (!webserv_utils::httpVersionIsSupported(_http_version))
		_error_status = 505;
}

void Request::_validateParsedHeaders()
{
	if (_error_status)
		return;
	if (_headers.empty() || _headers.count("Host") == 0 ||
	    (_headers.count("Content-Length") == 1 && _headers.count("Transfer-Encoding") == 1))
		_error_status = 400;
}

void Request::_validateParsedBody()
{
	if (_body.size() > static_cast<size_t>(_server->getBodySize()))
		_error_status = 413;
	else if (_body.size() != _content_length)
	{
		if (_body.size() > _content_length)
			_error_status = 413;
		else
			_error_status = 400;
	}
	// TODO what whith chunked request?
	else if (!_body.empty() && _headers.count("Content-Length") == 0)
		_error_status = 411;
//	if (_error_status)
//		_body.clear();
}


void Request::_validateMethod()
{
	if (!std::count(Webserv::implementedMethods.begin(), Webserv::implementedMethods.end(), _method))
	{
		_error_status = 501;
		return;
	}

	const std::vector<std::string> &allowed_methods(_server->getLocationlist().at(_server_location).methods);
	if (!std::count(allowed_methods.begin(), allowed_methods.end(), _method))
	{
		_error_status = 405;
		return;
	}
}

void Request::_setServer()
{
	_server = _potential_servers[0];
	std::vector<std::string> host;
	if (getValueOfHeader("Host", host) && !host.empty())
	{
		std::string host_name = host[0];

		for (std::vector<Server *>::const_iterator server_it = _potential_servers.begin();
		     server_it != _potential_servers.end(); server_it++)
		{
			std::vector<std::string> names = (*server_it)->getServerNames();
			for (std::vector<std::string>::const_iterator name_it = names.begin(); name_it != names.end(); name_it++)
			{
				if (*name_it == host_name)
					_server = *server_it;
			}
		}
	}

	if (DISPLAY_SERVER_FOR_REQUEST)
	{
		size_t i(0);

		for (std::vector<Server *>::const_iterator server_it = _potential_servers.begin();
		     server_it != _potential_servers.end(); server_it++)
		{
			if (_server == *server_it)
				break;
			i++;
		}
		std::cout << "************** Server ***************" << std::endl;
		std::cout << "Request will be served by server #" << i << std::endl;
		std::cout << "*****************************************" << std::endl << std::endl;
	}
}

void Request::_setLocation()
{
	std::string best_match("/");
	size_t size_best_match = best_match.size();

	UrlParser url_parsed(_request_target);

	std::map<std::string, t_location>::const_iterator cit;
	for (cit = _server->getLocationlist().begin(); cit != _server->getLocationlist().end(); ++cit)
	{
		size_t pos = url_parsed.path.find(cit->first);
		if (pos == 0 && cit->first.size() > size_best_match)
			best_match = cit->first;
	}
	_server_location = best_match;

	if (DISPLAY_LOCATION_FOR_REQUEST)
	{
		std::cout << "************** Url Parser ***************" << std::endl;
		std::cout << "[ UrlParser for " << url_parsed.resource << " ]" << std::endl;
		std::cout << url_parsed;
		std::cout << "best match for request_location is " << best_match << std::endl;
		std::cout << "*****************************************" << std::endl << std::endl;
	}
}

bool Request::retrieveRequest()
{
	ssize_t bytes;
	char buffer[BUFFER_SIZE] = {};
	std::string size;

	bytes = recv(_socket, buffer, BUFFER_SIZE, 0);
	if (bytes < 0)
		throw Request::readRequestException();
	if (bytes == 0)
		return false;
	for (ssize_t i(0); i < bytes; ++i)
		_request += buffer[i];

	if (DISPLAY_REQUEST)
	{
		std::cout << "****** Request on socket " << _socket << " (Received) ******" << std::endl;
		std::cout << _request << "[EOF]" << std::endl;
	}
	return true;
}

int Request::getSocket() const
{
	return _socket;
}

const std::string &Request::getMethod() const
{
	return _method;
}

const std::string &Request::getRequestTarget() const
{
	return _request_target;
}

const std::string &Request::getRequest() const
{
	return _request;
}

const std::string &Request::getHTTPVersion() const
{
	return _http_version;
}

const std::map<std::string, std::vector<std::string> > &Request::getHeaders() const
{
	return _headers;
}

const std::string &Request::getBody() const
{
	return _body;
}

bool Request::isChunkedRequest() const
{
	return _chunked_request;
}
int Request::getErrorStatus() const
{
	return _error_status;
}

size_t Request::getContentLength() const
{
	return _content_length;
}

bool Request::getValueOfHeader(const std::string &key, std::vector<std::string> &value) const
{
	std::map<std::string, std::vector<std::string> >::const_iterator it;
	it = _headers.find(key);
	if (it == _headers.end())
		return false;
	value = it->second;
	return true;
}

std::ostream &operator<<(std::ostream &o, const Request &rhs)
{
	o << "[ INFO ]" << std::endl;
	o << "_error_status: " << rhs.getErrorStatus() << std::endl;
	o << "_body.size(): " << rhs.getBody().size() << std::endl;
	o << "_content_length: " << rhs.getContentLength() << std::endl;
	o << "_chunked_request: " << std::boolalpha << rhs.isChunkedRequest() << std::endl;
	o << "[ PARSED ]" << std::endl;
	o << "_method: " << rhs.getMethod() << std::endl;
	o << "_request_target: " << rhs.getRequestTarget() << std::endl;
	o << "_http_version: " << rhs.getHTTPVersion() << std::endl;
	o << "_headers: " << std::endl;
	std::map<std::string, std::vector<std::string> >::const_iterator it = rhs.getHeaders().begin();
	for (; it != rhs.getHeaders().end(); ++it)
	{
		o << "   " << it->first << std::endl;
		std::vector<std::string>::const_iterator it2 = it->second.begin();
		for (; it2 != it->second.end(); ++it2)
			o << "      " << *it2 << std::endl;
	}
	o << "_body: " << std::endl;
	for (size_t i(0); i < rhs.getBody().size(); ++i)
		o << rhs.getBody().at(i);
	o << "[EOL]" << std::endl;
	o << "*****************************************" << std::endl << std::endl;
	return o;
}

void Request::getPotentialServers(std::vector<Server *> &server_list, struct sockaddr_in &addr)
{
	std::vector<struct sockaddr_in> end_points;

	for (std::vector<Server *>::iterator server_it = server_list.begin(); server_it != server_list.end(); server_it++)
	{
		end_points = (*server_it)->getEndPoints();
		for (std::vector<struct sockaddr_in>::iterator end_point_it = end_points.begin();
		     end_point_it != end_points.end(); end_point_it++)
		{
			if (end_point_it->sin_addr.s_addr == addr.sin_addr.s_addr && end_point_it->sin_port == addr.sin_port)
				_potential_servers.push_back(*server_it);
		}
	}
}