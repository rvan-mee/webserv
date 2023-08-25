/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HttpServer.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: cpost <cpost@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/07/27 10:17:59 by cpost         #+#    #+#                 */
/*   Updated: 2023/08/25 21:01:30 by rvan-mee      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <HttpServer.hpp>
#include <EventHandler.hpp>
#include <KqueueUtils.hpp>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdexcept> 
#include <sys/socket.h> 
#include <netinet/in.h> // sockaddr_in
#include <sys/event.h> // kqueue
#include <unistd.h> // close()
#include <iostream> 

#include <chrono>
#include <thread>

#define RESET   "\033[0m"
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define BLUE    "\033[34m"      /* Blue */


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
	if ( listen( _serverSocket, MAX_CONNECTIONS ) < 0)
		throw ( std::runtime_error( "Failed to start listening" )) ;

	this->setKqueue();

	/* The following loop will continue to execute as long as the condition in the while-loop 
	is true (always). This keeps the loop running repeatedly, allowing the server to remain 
	active and monitor events on the channel. */
	int	numEvents;
	while ( true )
	{
		/* The kevent function is used to wait and check for events on the this->kqueueFd 
		channel (a file system descriptor) with the event structure this->event. 
		The kevent function is set to a blocking mode because the KEVENT_FLAG_NONBLOCK is 
		not passed in the function call. This means the function will wait (block) until at 
		least one event occurs. If an event occurs, the details of the event are stored 
		in the array this->event, and the number of events is stored in the variable numEvents.*/
		numEvents = kevent( _kqueueFd, NULL, 0, _event, MAX_CONNECTIONS, NULL );
		if ( numEvents < 0 )
			throw ( std::runtime_error( "Server Terminated or Failed to wait for events" ) );

		/* A loop is executed over the received events in _event to process each one. 
		There are three possible events that can occur: 
		1. A client has disconnected.
		2. A new client has connected to the server.
		3. The server has to read from an fd.
		4. The server has to write to an fd. */
		for (int i = 0; i < numEvents; i++)
		{
			int	eventFd = _event[i].ident;
			// std::cout << "\n\nNew event on fd: " << eventFd << "\nfilter: "\
						// << (_event[i].filter == EVFILT_READ ? "READ" : "WRITE") << "\nflags: "\
						// << _event[i].flags << "\nfflags: " << _event[i].fflags << std::endl;

			/* If an event is detected on the serverSocket, it typically means 
			an incoming connection from a new client. In that case, the connection 
			is accepted, and the clientSocket is added to the _kqueueFd 
			channel to monitor it for read events (reading data from the client). */
			if ( eventFd == _serverSocket )
			{
				std::cout << "Adding client socket" << std::endl;
				int	clientSocket = accept( _serverSocket, NULL, NULL );
				if ( clientSocket < 0 )
					throw ( std::runtime_error( "Failed to accept connection" ) );

				/* Add the client socket to the kqueue */
				addKqueueEventFilter(_kqueueFd, clientSocket, EVFILT_READ);

				EventHandler*	newEvent = new EventHandler(clientSocket, _kqueueFd);
				_eventList.push_back(newEvent);
			}

			/* If the event is not on the serverSocket, we dont need to accept any new requests.
			This means that the we can perform a read or write operation on a different fd */
			int	eventIndex = this->getEventIndex(eventFd);
			if (eventIndex == -1)
				continue ;

			/* If the client had disconnected, close the connection */
			if ( _event[i].flags & EV_EOF ) {
				std::cout << RED "Closing client connection" RESET << std::endl;
				delete _eventList[eventIndex];
				_eventList.erase(_eventList.begin() + eventIndex);
				close(eventFd);
			}
			else if ( _event[i].filter == EVFILT_READ ) {
				std::cout << GREEN "Handling read event" RESET << std::endl;
				try {
					_eventList[eventIndex]->handleRead(eventFd);
				}
				catch(const std::exception& e) {
					std::cerr << e.what() << std::endl;
				}
			}
			else if ( _event[i].filter == EVFILT_WRITE) {
				std::cout << BLUE "Handling write event" RESET << std::endl;
				try {
					_eventList[eventIndex]->handleWrite(eventFd);
				}
				catch(const std::exception& e) {
					std::cerr << e.what() << std::endl;
				}
			}
		}
	}
}

int	HttpServer::getEventIndex( int fd )
{
	for (size_t i = 0; i < _eventList.size(); i++) {
		if (_eventList[i]->isEvent(fd))
			return (i);
	}
	return (-1);
}

/**
 * @brief Create a socket for the HTTP server to listen on.
 * @throw std::runtime_error if something goes wrong during the creation
*/
void	HttpServer::createSocket( void )
{
	int	reuse = 1;

	/* Create socket. The socket function returns an integer that is 
	used as a file descriptor. AF_INET = IPv4, SOCK_STREAM = TCP, 
	0 = default protocol */
	_serverSocket = socket( AF_INET, SOCK_STREAM, 0 );
	
	/* Check if socket creation was successful. If not, throw error */
	if ( _serverSocket < 0 )
		throw ( std::runtime_error( "Failed to create socket" ) );
	
	/* Allow reuse of a socket if it has recently been in use */
	if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0)
    	throw std::runtime_error( "Failed to set socket options" );

	/* Sets the socket to non-blocking */
	if (fcntl(_serverSocket, F_SETFL, O_NONBLOCK) < 0)
    	throw std::runtime_error( "Failed to set socket to non-blocking" );
}

/**
 * @brief Bind the socket to the port specified in the config
 * @param config The config object containing all the server information
 * @throw std::runtime_error if something goes wrong during the binding
*/
void	HttpServer::bindSocket( Config &config )
{
	_address.sin_family = AF_INET; // IPv4
	_address.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
	_address.sin_port = htons( config.getListen()[0] ); // Port to listen on

	/* Bind socket to port. If it fails, throw an error */
	if ( bind( _serverSocket, ( struct sockaddr * )&_address, sizeof( _address ) ) < 0 )
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
	_kqueueFd = kqueue();
	if ( _kqueueFd < 0 )
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
	EV_SET( _event, _serverSocket, EVFILT_READ, EV_ADD, 0, 0, NULL );

	// Kevent is used to register events with the kqueue and to monitor them for changes.
	if ( kevent( _kqueueFd, _event, 1, NULL, 0, NULL ) < 0 )
		throw ( std::runtime_error( "Failed to set event" ) );
}
