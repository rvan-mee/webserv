/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HttpServer.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: cpost <cpost@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/07/27 10:17:59 by cpost         #+#    #+#                 */
/*   Updated: 2023/08/08 14:49:33 by cpost         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "HttpServer.hpp" 
#include <stdexcept> 
#include <sys/socket.h> 
#include <netinet/in.h> // sockaddr_in
#include <sys/event.h> // kqueue
#include <unistd.h> // close()
#include <iostream> 

#include <chrono>
#include <thread>

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

//TODO: CODAM will be migrating to Linux soon, 
//so kqueue will have to be changed to epoll maybe.
	this->setKqueue();

	/* The following loop will continue to execute as long as the condition in the while-loop 
	is true (always). This keeps the loop running repeatedly, allowing the server to remain 
	active and monitor events on the channel. */
	int			numEvents;
	while ( true )
	{
		/* The kevent function is used to wait and check for events on the this->kqueueFd 
		channel (a file system descriptor) with the event structure this->event. 
		The kevent function is set to a blocking mode because the KEVENT_FLAG_NONBLOCK is 
		not passed in the function call. This means the function will wait (block) until at 
		least one event occurs. If an event occurs, the details of the event are stored 
		in the array this->event, and the number of events is stored in the variable numEvents.*/
		numEvents = kevent( this->kqueueFd, NULL, 0, this->event, MAX_CONNECTIONS, NULL );
		if ( numEvents < 0 )
			throw ( std::runtime_error( "Server Terminated or Failed to wait for events" ) );
		
		/* A loop is executed over the received events in this->event to process each one. 
		There are three possible events that can occur: 
		1. A client has disconnected.
		2. A new client has connected to the server.
		3. An existing client has sent data to the server. */
		for (int i = 0; i < numEvents; i++)
		{
			int	eventFd = this->event[i].ident;

			/* If an event is detected on the serverSocket, it typically means 
			an incoming connection from a new client. In that case, the connection 
			is accepted, and the clientSocket is added to the this->kqueueFd 
			channel to monitor it for read events (reading data from the client). */
			if ( eventFd == this->serverSocket )
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

			/* If the event is not on the serverSocket, it is on a clientSocket.
			This means that the client has sent data to the server. */
			else if ( event[i].filter & EVFILT_READ )
			{
				/* Create a buffer to store the data received from the client.
				The initial size of the buffer is 1024 bytes. It will be resized later
				if the data received from the client (requestSize) is larger than 
				1024 bytes. */
				std::vector<char>	buffer( 1024 );
				size_t				requestSize = 0;

				/* With the following loop, the data is read from the client socket until 
				there is no more data to read. If there is more data to read 
				(bytesRead == 0), the loop is ended. */
				while ( true )
				{
					// Read the data from the client socket into the buffer
					ssize_t	bytesRead = recv( eventFd, buffer.data(), buffer.size(), 0 );

					// Increase the request size by the number of bytes read
					requestSize += bytesRead;

					// Check if there has been an error reading from the client socket
					if ( bytesRead < 0 )
						throw ( std::runtime_error( "Failed to read from client socket" ) );

					// If there is no more data to read, break out of the loop
					else if ( bytesRead == 0 )
						break ;

					// If the buffer is full, resize it
					else if ( requestSize >= buffer.size() )
						buffer.resize( buffer.size() * 2 );
				}
// Print the request received from the client (FOR TESTING)
std::cout << "Received request: " << buffer.data() << std::endl;

			/* If the client had disconnected, close the connection */
			if ( event[i].flags & EV_EOF )
				close( eventFd );

				
				/* TODO 1:
				The data received from the client is now stored in the buffer.
				This buffer has to be parsed to extract the request information.
				A function will be created to do this.
				*/ 
				
				/* TODO 2:
				After the request has been parsed, the response has to be created.
				*/
				
				close ( eventFd );
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
