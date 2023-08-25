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
	str += "HTTP/1.1 ";
	str += std::to_string(_status_code);
	str += " ";
	str += _reason_phrase;
	str += "\r\n";
	str += "Content-Type: ";
	str += _content_type;
	str += "\r\n\r\n";
	// str+= _message_body;
	std::ifstream t("../../src/http/mytest/index.html");
	std::stringstream buffer;
	buffer << t.rdbuf();
	str += buffer.str();
	str += "\r\n";
	return (str);
}
