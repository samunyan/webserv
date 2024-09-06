#ifndef URLPARSER_HPP
#define URLPARSER_HPP

#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <utility>

class UrlParser
{
  public:
	explicit UrlParser(const std::string &url);
	virtual ~UrlParser();
	std::string resource;
	std::string path;
	std::string query_string;
	std::string directory;
	std::string file_name;
	std::string file_extension;
	std::map<std::string, std::string> query_params;
	static bool queryStringToParams(const std::string &string, std::map<std::string, std::string> &params);
	std::string getFileType(std::string);

  private:
	UrlParser();
	UrlParser(const UrlParser &src);

	static std::map<std::string, std::string> _all_file_types;
};

std::ostream &operator<<(std::ostream &o, const UrlParser &rhs);

#endif
