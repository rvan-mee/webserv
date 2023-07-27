/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HttpServer.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: cpost <cpost@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/07/27 10:17:59 by cpost         #+#    #+#                 */
/*   Updated: 2023/07/27 11:10:47 by cpost         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "HttpServer.hpp" 
#include <stdexcept> 
#include <sys/socket.h> 
#include <netinet/in.h>

/******************************
* Constructors & Destructors
*****************************/

/**
 * @brief Construct a new Http Server:: Http Server object
*/
HttpServer::HttpServer( void )
{
}

/**
 * @brief Destroy the Http Server:: Http Server object
*/
HttpServer::~HttpServer()
{
}

/******************************
* Init server
*****************************/

/**
 * @brief Setup the HTTP server
 * @param config The config object containing all the server information
 * @throw std::runtime_error if something goes wrong during the setup
*/
void	HttpServer::initServer( Config &config )
{
	/* Create socket */
	this->createSocket();

	/* Bind socket to port */
	this->bindSocket( config );

	/* Start listening. MAX_CONNECTIONS is defined in HttpServer.hpp */
	if ( listen( this->serverSocket, MAX_CONNECTIONS ) < 0)
		throw ( std::runtime_error( "Failed to start listening" )) ;

	/* TODO: Work from here. Create a loop that accepts connections and handles them
	according to the HTTP protocol. Probably have to use Kqueue for this */
}

/**
 * @brief Create a socket for the HTTP server to listen on.
 * @throw std::runtime_error if something goes wrong during the creation
*/
void	HttpServer::createSocket( void )
{
	/* Create socket. The socket function returns an integer that is 
	used as a file descriptor. AF_INET = IPv4, SOCK_STREAM = TCP, 
	0 = default protocol*/
	this->serverSocket = socket( AF_INET, SOCK_STREAM, 0 );
	
	/* Check if socket creation was successful. If not, throw error */
	if ( this->serverSocket < 0 )
		throw ( std::runtime_error( "Failed to create socket" ) );
}

/**
 * @brief Bind the socket to the port specified in the config
 * @param config The config object containing all the server information
 * @throw std::runtime_error if something goes wrong during the binding
*/
void	HttpServer::bindSocket( Config &config )
{
	this->address.sin_family = AF_INET; // IPv4
	this->address.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
	this->address.sin_port = htons( config.getListen()[0] ); // Port to listen on

	/* Bind socket to port. If it fails, throw an error */
	if ( bind( this->serverSocket, ( struct sockaddr * )&this->address, sizeof( this->address ) ) < 0 )
		throw ( std::runtime_error( "Failed to bind socket to port" ) );
}