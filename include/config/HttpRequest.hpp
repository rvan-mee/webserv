/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HttpRequest.hpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: cpost <cpost@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/07/27 10:17:28 by cpost         #+#    #+#                 */
/*   Updated: 2023/09/04 16:29:12 by dkramer       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP


#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>
#include "HttpResponse.hpp"
/**
 * @brief HtppRequest class to parse the request and store the data in the class variables
 * 
 */
class HttpRequest
{
	public:
		enum requestType { GET, POST, DELETE };
		std::string    parseRequestandGiveReponse( std::vector<char> buffer, Config &config );
		void	isRequestLine(std::string line, HttpResponse &response);
		void	isHeader(std::string line, HttpResponse &response);
		requestType	getMethod();
		void	setMethod(requestType method);
		void	setURI(std::string target);
		void	setContentType(std::string contentType);
		void	setHost(std::string host);
		void    addLineToBody(std::string line);
		void    printAll();
		void    parseCgiRequest(HttpResponse &response);
	private:
		requestType	_request_method;
		std::string _request_URI;
		std::string	_message_body;
		std::string	_content_type;
		std::string	_host;
};

#endif
