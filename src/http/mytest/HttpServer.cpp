#include "HttpServer.hpp" 

HttpServer::requestType	HttpServer::getMethod()
{
    if (_request_method == GET)
        std::cout << "GET";
    else if (_request_method == POST)
        std::cout << "POST";
    else if (_request_method == DELETE)
        std::cout << "DELETE";
    return _request_method;
}

void	HttpServer::setMethod(requestType method)
{
    _request_method = method;
}

void	HttpServer::setURI(std::string target)
{
    if (target.empty())
        throw ( std::runtime_error( "no target in request found" ) );
    _request_URI = target;
}

void	HttpServer::setContentType(std::string contentType)
{
    if (contentType.empty())
        throw ( std::runtime_error( "no content-type in request found" ) );
    _content_type = contentType;
}

void	HttpServer::setHost(std::string host)
{
    if (host.empty())
        throw ( std::runtime_error( "no host in request found" ) );
    _host = host;
}

void    HttpServer::addLineToBody(std::string line)
{
    _message_body += line;
    _message_body += "\n";
}

void    HttpServer::printAll()
{
    std::cout << "Method: " << _request_method << std::endl;
    std::cout << "URI: " << _request_URI << std::endl;
    std::cout << "Content-Type: " << _content_type << std::endl;
    std::cout << "Host: " << _host << std::endl;
    std::cout << "Body: " << _message_body << std::endl;

}
