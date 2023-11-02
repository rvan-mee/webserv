/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HttpRequest.hpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: cpost <cpost@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/07/27 10:17:28 by cpost         #+#    #+#                 */
/*   Updated: 2023/09/25 15:09:46 by rvan-mee      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP


#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <HttpResponse.hpp>
#include <cstring>
#include <EventPoll.hpp>
#include <CgiHandler.hpp>
#include <EventPoll.hpp>

class CgiHandler;

enum requestType	{ GET, POST, DELETE };

/**
 * @brief HttpRequest class to parse the request and store the data in the class variables
 * 
 */
class HttpRequest
{
	public:
		HttpRequest(CgiHandler& cgi, EventPoll& poll, int socketFd) : _cgi(cgi), _poll(poll), _socketFd(socketFd) {}

		std::string			parseRequestAndGiveResponse(std::vector<char> buffer, Server server);
		void				isRequestLine(std::string line, HttpResponse &response);
		void				isHeader(std::string line, HttpResponse &response);
		requestType			getMethod();
		void				setMethod(requestType method);
		void				setURI(std::string target);
		void				setContentType(std::string contentType);
		void				setHost(std::string host);
		void				addLineToBody(std::string line);
		void				printAll();
		void				parseCgiRequest(HttpResponse &response, Server server, bool& isCgiRequest);
		void				parseGetRequest(HttpResponse &response, Server server);
		void				parsePostRequest(HttpResponse &response, Server server, bool& isCgiRequest);
		void				parseDeleteRequest(HttpResponse &response, Server server);

		void			setContentLength( long contentLength) { _contentLength = contentLength; };

		requestType		getRequestType( void ) { return(_request_method); };
		std::string&	getUri( void ) { return(_request_URI); };
		std::string&	getBody( void ) { return(_message_body); };
		std::string&	getContentType( void ) { return(_content_type); };
		std::string&	getHost( void ) { return(_host); };
		long			getContentLength( void ) { return(_contentLength); };

	private:
		HttpRequest();

		requestType	_request_method;
		std::string _request_URI;
		std::string	_message_body;
		std::string	_content_type;
		std::string	_host;
		long		_contentLength;
		CgiHandler&	_cgi;
		EventPoll&	_poll;
		int			_socketFd;
};

#endif
