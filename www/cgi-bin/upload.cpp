#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdio>

int ft_stoi(std::string);
std::string successPage(std::string);
std::string errorPage();
std::string noFile();
std::string getFilename(std::string &);
std::string getBoundary(std::string &);
int findStartOfFile(std::string &, std::string &);

int main(int argc, char **argv, char **env)
{
	std::ifstream infile;
	std::ofstream outfile;
	std::string file, filename, boundary, message;
	std::string socket(argv[0]);
	char c;
	int pos, sock;

	infile.open("tmp", std::ifstream::in | std::ifstream::binary);

	if (infile.fail())
		message = errorPage();
	else
	{
		while (!infile.eof())
		{
			infile.read(&c, 1);
			file.append(1, c);
		}
		infile.close();

		sock = ft_stoi(&socket[socket.find_first_of("=") + 1]);

		boundary = getBoundary(file);
		boundary.insert(0, "--");
		filename = getFilename(file);
	}
	if (!filename.size())
		message = noFile();
	else
	{
		filename.insert(0, "www/uploads/");
		pos = findStartOfFile(file, boundary);
		file.erase(0, pos);
		file.erase(file.find(boundary) - 4, file.npos);

		outfile.open(filename, std::ofstream::out | std::ofstream::trunc | std::ofstream::binary);

		if (outfile.fail())
			message = errorPage();
		else
		{
			outfile << file;
			outfile.close();
			filename = &filename[filename.rfind('/') + 1];
			filename.append("/download.cgi");
			message = successPage(filename);
		}
	}

	send(sock, message.c_str(), message.size(), 0);

	return 0;
}

int ft_stoi(std::string str)
{
	int result = 0;
	int size = str.size();

	for (int i = 0; i < size; i++)
		result = result * 10 + str[i] - '0';

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

std::string successPage(std::string filename)
{
	std::string html, message = "HTTP/1.1 201 Created\r\nConnection: keep-alive\r\nContent-Length: \r\nContent-Type: text/html\r\n\r\n";
	std::stringstream ss;
	ss << "<!DOCTYPE html>"
	      "<html>\n"
	      "<head>"
		  	"<meta charset=\"UTF-8\">\n"
   		  	"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
		  	"<link rel=\"shortcut icon\" type=\"image/x-icon\" href=\"/assets/favicon.ico\">\n"
    	  	"<link rel=\"stylesheet\" type=\"text/css\" href=\"/assets/stylesheet.css\">\n"
		  	"<title>Upload successful</title>\n"
	      "</head>\n"
	             "<body>\n"
	             "<center><h1>"
				 "You file has been successfully uploaded !\n\n</h1>"
				 "<a href=\"" + filename + "\" download=\"" + filename.substr(0, filename.find('/')) + "\">Download it back</a>"
	      "</center>\n"
	             "</body>\n"
	             "</html>";
	html = ss.str();
	message.insert(message.find("\r\nContent-Type"), ft_to_string(html.size()));
	message.append(html);
	return message;
}

std::string errorPage()
{
	std::string html, message = "HTTP/1.1 500 Internal Server Error\r\nConnection: keep-alive\r\nContent-Length: \r\nContent-Type: text/html\r\n\r\n";
	std::stringstream ss;
	ss << "<!DOCTYPE html>"
	      "<html>\n"
	      "<head>"
		  	"<title>Upload failed</title>\n"
	      "</head>\n"
	             "<body>\n"
	             "<center><h1>"
				 "500 Internal Server Error"
	      "</h1></center>\n"
	             "</body>\n"
	             "</html>";
	html = ss.str();
	message.insert(message.find("\r\nContent-Type"), ft_to_string(html.size()));
	message.append(html);
	return message;
}

std::string noFile()
{
	std::string html, message = "HTTP/1.1 400 Bad Request\r\nConnection: keep-alive\r\nContent-Length: \r\nContent-Type: text/html\r\n\r\n";
	std::stringstream ss;
	ss << "<!DOCTYPE html>"
	      "<html>\n"
	      "<head>"
		  	"<title>No file</title>\n"
	      "</head>\n"
	             "<body>\n"
	             "<center><h1>"
				 "400 Bad Request"
	      "</h1></center>\n"
	             "</body>\n"
	             "</html>";
	html = ss.str();
	message.insert(message.find("\r\nContent-Type"), ft_to_string(html.size()));
	message.append(html);
	return message;
}

std::string getFilename(std::string &file)
{
	std::string result = "";

	result = &file[file.find("filename=\"") + 10];
	result = result.substr(0, result.find('"'));

	return result;
}

std::string getBoundary(std::string &file)
{
	std::string result;

	result = &file[file.find("boundary=") + 9];
	result = result.substr(0, result.find("\r\n"));

	return result;
}

int findStartOfFile(std::string &file, std::string &boundary)
{
	int pos = 0;

	pos = file.find(boundary) + boundary.size();
	pos = file.find("\r\n\r\n", pos) + 4;

	return pos;
}