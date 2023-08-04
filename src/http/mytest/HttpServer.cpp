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
