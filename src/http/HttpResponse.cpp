#include <HttpResponse.hpp>
#include <Server.hpp>

HttpResponse::HttpResponse()
{
	_status_code = 0;
	_reason_phrase = "";
	_content_type = "";
	_message_body = "";
}

void HttpResponse::setContentType(std::string contentType)
{
	// if (contentType.empty())
	// 	throw(std::runtime_error("no content-type in response found"));
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

bool replace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

void	 HttpResponse::setMessageBody( Server server )
{
	std::map<std::string, int>::iterator it;
	std::string errorPage = server.getErrorPage(_status_code);
	std::cout << "errorPage: " << errorPage << std::endl;
	if (errorPage.empty() && _status_code != 200)
	{
		std::ifstream error("default_error.html"); //taking file as inputstream
		std::string e;
		if (error.is_open())
		{
			std::ostringstream ss;
			ss << error.rdbuf(); // reading data
			e = ss.str();
			replace(e, "[ERRORCODE]", std::to_string(_status_code));
			replace(e, "[REASONPHRASE]", _reason_phrase);
			_message_body = e;
		}
		else
		{
			std::cout << "Error opening file";
		}
	}
	else if (_status_code != 200)
	{
		std::ifstream error(server.getRoot() + errorPage); //taking file as inputstream
		std::string e;
		if (error.is_open())
		{
			std::ostringstream ss;
			ss << error.rdbuf(); // reading data
			e = ss.str();
			_message_body = e;
		}
		else
		{
			std::cout << "Error opening file";
		}
	}
	else
	{
		std::ifstream f("src/http/index.html"); //taking file as inputstream
		std::string s;
		if (f.is_open())
		{
			std::ostringstream ss;
			ss << f.rdbuf(); // reading data
			s = ss.str();
			_message_body = s;
		}
		else
		{
			std::cout << "Error opening file";
		}
	}
}

std::string HttpResponse::buildResponse( Server server)
{
	std::string str;
	if (!_status_code)
		_status_code = 200;
	if (_reason_phrase.empty())
		_reason_phrase = "OK";
	if (_content_type.empty())
		_content_type = "text/html";
	setMessageBody(server);
	str += "HTTP/1.1 ";
	str += std::to_string(_status_code);
	str += " ";
	str += _reason_phrase;
	str += "\r\n";
	str += "Content-Type: ";
	str += _content_type;
	if (!server.getServerNames()[0].empty())
		str += "\r\nServer: " + server.getServerNames()[0];
	str += "\r\n\r\n";
	// str += s;
	str+= _message_body;
	str += "\r\n";
	// str+= "HTTP/1.1 500 nope\r\nContent-Length: 88\r\nContent-Type: text/html\r\nConnection: keep-alive\r\n\r\n<html>\n<body>\n<h1>Hello,
	return (str);
}
