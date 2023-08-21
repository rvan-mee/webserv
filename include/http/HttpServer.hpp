/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HttpServer.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: cpost <cpost@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/07/27 10:17:28 by cpost         #+#    #+#                 */
/*   Updated: 2023/08/08 13:59:24 by cpost         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPSERVER_HPP
# define HTTPSERVER_HPP

# define MAX_CONNECTIONS 100 // Used in HttpServer.cpp -> initServer()

#include <Config.hpp>
#include <netinet/in.h> // sockaddr_in
#include <sys/event.h> // kqueue

class HttpServer
{
private:

	int				serverSocket;
	sockaddr_in 	address;
	int				kqueueFd;
	struct kevent	event[ MAX_CONNECTIONS + 1 ];

public:

	/******************************
	* Constructors & Destructors
	*****************************/

	HttpServer( void );
	~HttpServer();

	/******************************
	* Server init
	*****************************/

	void	initServer( Config &config );
	void	createSocket( void );
	void	bindSocket( Config &config );
	void	setKqueue( void );
};

#endif
