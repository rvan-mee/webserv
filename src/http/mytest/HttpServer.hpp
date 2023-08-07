/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HttpServer.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: cpost <cpost@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/07/27 10:17:28 by cpost         #+#    #+#                 */
/*   Updated: 2023/08/04 13:28:41 by dkramer       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPSERVER_HPP
# define HTTPSERVER_HPP

# define MAX_CONNECTIONS 100 // Used in HttpServer.cpp -> initServer()

#include <netinet/in.h> // sockaddr_in
// #include <sys/event.h> // kqueue
#include <string>
#include <vector>
#include <iostream>
#include <sstream>


class HttpServer
{
	public:
		enum requestType { GET, POST, DELETE };
		void    parseRequest( std::vector<char> buffer );
		void	isRequestLine(std::string line);
		requestType	getMethod();
		void	setMethod(requestType method);
		void	setURI(std::string target);
	private:
		requestType	_request_method;
		std::string _request_URI;
		std::string	_message_body;
};

#endif
