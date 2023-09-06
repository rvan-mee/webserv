#include "HttpResponse.hpp"

void HttpResponse::setContentType(std::string contentType)
{
	if (contentType.empty())
		throw(std::runtime_error("no content-type in request found"));
	_content_type = contentType;
}

void HttpResponse::addLineToBody(std::string line)
{
	_message_body += line;
	_message_body += "\n";
}

void HttpResponse::printAll()
{
	std::cout << "Reason Phrase: " << _reason_phrase << std::endl;
	std::cout << "status code: " << _status_code << std::endl;
	std::cout << "Content-Type: " << _content_type << std::endl;
	std::cout << "Body: " << _message_body << std::endl;
}

void HttpResponse::setError(int statusCode, std::string reasonPhrase)
{
	_status_code = statusCode;
	_reason_phrase = reasonPhrase;
}

std::string HttpResponse::buildResponse()
{
	std::string str;
	_status_code = 200;
	_reason_phrase = "OK";
	_content_type = "text/html";
	_message_body = "<html>\n<body>\n<h1>test</h1>\n</body>\n</html>\n";
	str += "HTTP/1.1 ";
	str += std::to_string(_status_code);
	str += " ";
	str += _reason_phrase;
	str += "\r\n";
	str += "Content-Type: " + _content_type + "\r\n";
	std::ifstream f("src/http/index.html"); //taking file as inputstream
	std::string s;
	if (f.is_open())
	{
		std::ostringstream ss;
		ss << f.rdbuf(); // reading data
		s = ss.str();
	}
	else
	{
		std::cout << "Error opening file";
	}
	str += "Content-Length: ";
	str += std::to_string(s.size());
	str += "\r\n\r\n";
	str += s;
	// str+= _message_body;
	// str += "\r\n";
	// str+= "HTTP/1.1 500 nope\r\nContent-Length: 88\r\nContent-Type: text/html\r\nConnection: keep-alive\r\n\r\n<html>\n<body>\n<h1>Hello,
	return (str);
}
