#include <HttpResponse.hpp>
#include <Server.hpp>

/**
 * @brief Construct a new Http Response:: Http Response object
 * initialize all the variables to empty.
*/
HttpResponse::HttpResponse()
{
	_status_code = 0; 
	_reason_phrase = ""; 
	_content_type = ""; 
	_message_body = ""; 
	_redirect = "";
}

void HttpResponse::setContentType(std::string contentType)
{
	// if (contentType.empty())
	// 	throw(std::runtime_error("no content-type in response found"));
	_content_type = contentType;
}

/**
 * @brief Add a line to the message body
 * @param line The line to add
*/
void HttpResponse::addLineToBody(std::string line)
{
	_message_body += line;
	_message_body += "\n";
}

/**
 * @brief Print all the data in the class
*/
void HttpResponse::printAll()
{
	std::cout << "Reason Phrase: " << _reason_phrase << std::endl;
	std::cout << "status code: " << _status_code << std::endl;
	std::cout << "Content-Type: " << _content_type << std::endl;
	std::cout << "Body: " << _message_body << std::endl;
}

/**
 * @brief Set the error object.
 * @param statusCode The status code
 * @param reasonPhrase The corresponding explanation for the status code
*/
void HttpResponse::setError(int statusCode, std::string reasonPhrase)
{
	_status_code = statusCode;
	_reason_phrase = reasonPhrase;
}

/**
 * @brief Set the redirect object.
 * @param redirect The redirect location
*/
void HttpResponse::setRedirect( std::string redirect )
{
	this->_redirect = redirect;
}

bool replace(std::string& str, const std::string& from, const std::string& to) 
{
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

/**
 * @brief Build the response string
 * @param server The server object with all the server information
 * @return std::string The response string
*/
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
		str += "\r\nServer: " + server.getServerNames()[0] + "\r\n";
	str += "Content-Length: ";
	str += std::to_string(_message_body.size());
	str += "\r\n\r\n";
	str += _message_body;
	str += "\r\n";
	return (str);
}
