#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <iostream>
#include <map>
#include "Request.hpp"
#include <sys/stat.h>
#include "../utils/UrlParser.hpp"
#include <cstdio>

class Request;

class Response
{
  public:
	Response(Request *request);
	virtual ~Response();
	void handleRequest();
	void buildMessage();
	void sendMessage();
	const std::string &getResourcePath() const;
	friend std::ostream &operator<<(std::ostream &o, const Response &rhs);
	class sendResponseException : public std::exception
	{
	  public:
		virtual const char *what() const throw()
		{
			return "Error while sending response. Client dismissed.";
		}
	};

  private:
	Response();
	Response(const Response &src);
	Response &operator=(const Response &rhs);
	static std::map<int, std::string> _all_status_codes;
	Request *_request;
	int _status_code;
	std::string _http_version;
	std::string _status_line;
	std::map<std::string, std::vector<std::string > > _headers;
	size_t _content_length;
	std::string _headersAsString;
	std::string _body;
	std::string _message;
	static std::map<std::string, std::string> _methodMatches;

	bool _chunked_response;
	bool _dir_listing;
	bool _handled_by_CGI;

	std::string _resource_path;
	void _setResourcePath();

	void _buildHeaders();
	void _buildStatusLine();
	bool _retrieveResponseBody(const std::string &path);

	bool _buildDirListing();

	void _buildErrorBody();
	void _buildDefaultErrorBody();
	bool _buildCustomErrorBody();

	bool _handleCgi();
	void _doGet();
	void _doPost();
	void _doDelete();

	void _chunkResponse();
};

#endif
