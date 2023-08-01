/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HttpServer.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: cpost <cpost@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/07/27 10:17:59 by cpost         #+#    #+#                 */
/*   Updated: 2023/08/01 16:46:49 by cpost         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "HttpServer.hpp" 
#include <stdexcept> 
#include <sys/socket.h> 
#include <netinet/in.h> // sockaddr_in
#include <sys/event.h> // kqueue
#include <unistd.h> // close()

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
	according to the HTTP protocol. Probably have to use Kqueue for this.
	see: https://dev.to/frevib/a-tcp-server-with-kqueue-527
	 */
//TODO: CODAM will be migrating to Linux soon, 
//so kqueue will have to be changed to epoll.
	this->setKqueue();

	int		numEvents;
	while ("The world keeps turning")
	{
		/* Wait for events to be triggered. numEvents is the number 
		of events that have been triggered. kevent() returns 0 if 
		no events have been triggered. */
		numEvents = kevent( this->kqueueFd, NULL, 0, this->event, MAX_CONNECTIONS, NULL );
		if ( numEvents < 0 )
			throw ( std::runtime_error( "Failed to wait for events" ) );
		
		/* If an event has been triggered, handle it */
		for (int i = 0; i < numEvents; i++)
		{
			int	eventFd = this->event[i].ident;

			/* If the client had disconnected, close the connection */
			if ( event[i].flags & EV_EOF )
				close( eventFd );

			/* If the event is the server socket, accept the connection */
			else if ( eventFd == this->serverSocket )
			{
				int	clientSocket = accept( this->serverSocket, NULL, NULL );
				if ( clientSocket < 0 )
					throw ( std::runtime_error( "Failed to accept connection" ) );
					
				/* Add the client socket to the kqueue */
				struct kevent	evSet;
				EV_SET( &evSet, clientSocket, EVFILT_READ, EV_ADD, 0, 0, NULL );
				if ( kevent( this->kqueueFd, &evSet, 1, NULL, 0, NULL ) < 0 )
					throw ( std::runtime_error( "Failed to add client socket to kqueue" ) );
			}

			else
			{
				// 
				int clientSocket = this->event[i].ident;
				//TODO: Handle request
				//this->handleRequest( clientSocket );
			}
		}
	}
}

/**
 * @brief Create a socket for the HTTP server to listen on.
 * @throw std::runtime_error if something goes wrong during the creation
*/
void	HttpServer::createSocket( void )
{
	/* Create socket. The socket function returns an integer that is 
	used as a file descriptor. AF_INET = IPv4, SOCK_STREAM = TCP, 
	0 = default protocol */
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

/**
 * @brief Set up the kqueue for the HTTP server to listen on.
 * @throw std::runtime_error if something goes wrong during the setup
 */
void	HttpServer::setKqueue( void )
{	
	/* Create kqueue. Kqueue is a kernel event notification mechanism.
	It allows the user to monitor multiple file descriptors to see if
	they are ready for reading or writing.
	 */
	this->kqueueFd = kqueue();
	if ( this->kqueueFd < 0 )
		throw ( std::runtime_error( "Failed to create kqueue" ) );

	/* Set event. The parameters of EV_SET are as follows:
	1. The first parameter is the event to be modified. 
	In this case, it is the event struct in the HttpServer class.
	2. The second parameter is the file descriptor to be monitored, 
	in this case the server socket. 
	3. The third parameter is the type of event to be monitored 
	for, in this case read events. 
	4. The fourth parameter is the action to be taken for the event,
	in this case add the event to the kqueue. 
	5. The fifth parameter is the filter flag for the event type. 
	6. The sixth parameter is the flags for the event type.
	7. The seventh parameter is the data to be passed to the callback function
	in the event of an event. Here it is NULL because we don't need to pass
	any data to the callback function.
	*/
	EV_SET( this->event, this->serverSocket, EVFILT_READ, EV_ADD, 0, 0, NULL );

	// Kevent is used to register events with the kqueue and to monitor them for changes.
	if ( kevent( this->kqueueFd, this->event, 1, NULL, 0, NULL ) < 0 )
		throw ( std::runtime_error( "Failed to set event" ) );
}
