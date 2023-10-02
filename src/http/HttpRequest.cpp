#include <HttpRequest.hpp>

HttpRequest::requestType	HttpRequest::getMethod()
{
    if (_request_method == GET)
        std::cout << "GET";
    else if (_request_method == POST)
        std::cout << "POST";
    else if (_request_method == DELETE)
        std::cout << "DELETE";
    return _request_method;
}

void	HttpRequest::setMethod(requestType method)
{
    _request_method = method;
}

void	HttpRequest::setURI(std::string target)
{
    _request_URI = target;
}

void	HttpRequest::setContentType(std::string contentType)
{
    if (contentType.empty())
        throw ( std::runtime_error( "no content-type in request found" ) ); // dont know if content-type is mandatory
    _content_type = contentType;
}

void	HttpRequest::setHost(std::string host)
{
    if (host.empty())
        throw ( std::runtime_error( "no host in request found" ) ); // dont know if host is mandatory
    _host = host;
}

void    HttpRequest::addLineToBody(std::string line)
{
    _message_body += line;
    _message_body += "\n";
}

void    HttpRequest::printAll()
{
    std::cout << "Method: " << _request_method << std::endl;
    std::cout << "URI: " << _request_URI << std::endl;
    std::cout << "Content-Type: " << _content_type << std::endl;
    std::cout << "Host: " << _host << std::endl;
    std::cout << "Body: " << _message_body << std::endl;

}
