/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HttpServer.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: cpost <cpost@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/07/27 10:17:59 by cpost         #+#    #+#                 */
/*   Updated: 2023/09/25 15:11:32 by rvan-mee      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <HttpServer.hpp>
#include <ClientHandler.hpp>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdexcept> 
#include <sys/socket.h> 
#include <netinet/in.h> // sockaddr_in
#include <poll.h>
#include <unistd.h> // close()
#include <iostream> 
#include <algorithm>

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

void	HttpServer::closeServerSockets( void )
{
	for (size_t i = 0; i < _serverSockets.size(); i++) {
		close(_serverSockets[i]);
	}
}

bool	HttpServer::isServerSocket( int fd )
{
	for (size_t i = 0; i < _serverSockets.size(); i++) {
		if (_serverSockets[i] == fd)
			return ( true );
	}
	return ( false );
}

void HttpServer::removeClient(int eventIndex, int eventFd)
{
	std::cout << RED "Closing client connection" RESET << std::endl;
	delete _eventList[eventIndex];
	_eventList.erase(_eventList.begin() + eventIndex);
	_poll.removeEvent(eventFd, POLLIN | POLLRDHUP);
	_poll.removeEvent(eventFd, POLLOUT);
	close(eventFd);
}

/**
 * @brief Setup the HTTP server
 * @param config The config object containing all the server information
 * @throw std::runtime_error if something goes wrong during the setup
*/
void	HttpServer::initServer( Config &config )
{
	/* Set all the ports we need to listen to */
	this->setPorts( config );

	/* Create socket */
	this->createSockets();

	/* Bind socket to port */
	this->bindSockets();

	/* Start listening to the sockets for connections */
	this->listenToSockets();

	/* Add all sockets to the poll list */
	this->pollSockets();

	/* The following loop will continue to execute as long as the condition in the while-loop 
	is true (always). This keeps the loop running repeatedly, allowing the server to remain 
	active and monitor events on the channel. */
	int		ready;
	while ( true )
	{
		_poll.updateEventList();
		pollfd*	events = _poll.getEvents().data();
		size_t	numEvents = _poll.getEvents().size();

		// std::cout << "List before poll: " << std::endl;
		// _poll.printList();

		ready = poll(events, numEvents, -1);
		if (ready < 0) {
			closeServerSockets();
			throw ( std::runtime_error( "Failed to wait for events" ) );
		}


		// std::cout << "Ready: " << ready << std::endl;

		// _poll.printList();
		for (size_t i = 0; i < numEvents; i++) {
			// This event does not contain an fd that is ready
			if (events[i].revents == 0)
				continue ;

			int eventFd = events[i].fd;

			if ( this->isServerSocket(eventFd) )
			{
				int	clientSocket = accept( eventFd, NULL, NULL );
				if ( clientSocket < 0 ) {
					closeServerSockets();
					throw ( std::runtime_error( "Failed to accept connection" ) );
				}

				/* Add the client socket to the poll list */
				_poll.addEvent(clientSocket, POLLIN | POLLRDHUP);

				ClientHandler*	newEvent = new ClientHandler(clientSocket, _poll, config);
				_eventList.push_back(newEvent);
				continue ;
			}

			/* If the event is not on the serverSocket, we dont need to accept any new requests.
			This means that the we can perform a read or write operation on a different fd */
			int	eventIndex = this->getEventIndex(eventFd);
			if (eventIndex == -1)
				continue ;

			try {
				/* If the client had disconnected, close the connection */
				if ( events[i].revents & POLLHUP ) { // TODO: still need to watch out that it is not a pipe closure?
					this->removeClient(eventIndex, eventFd);
				}
				if ( events[i].revents & POLLERR ) {
					std::cout << "POLLERR CAUGHT" << std::endl;
				}
				if ( events[i].revents & POLLIN ) {
					std::cout << GREEN "Handling read event" RESET << std::endl;
					if (events[i].revents & POLLRDHUP)
						_eventList[eventIndex]->setHup();
					_eventList[eventIndex]->handleRead(eventFd, _poll);
				}
				if ( events[i].revents & POLLRDHUP && _eventList[eventIndex]->doneWithRequest()) {
					this->removeClient(eventIndex, eventFd);
				}
				if ( events[i].revents & POLLOUT ) {
					std::cout << BLUE "Handling write event" RESET << std::endl;
					_eventList[eventIndex]->handleWrite(eventFd);
				}
			} catch (const std::exception& e) {
				std::cerr << e.what() << '\n';
			}
		}
		// break;
	}
}

void	HttpServer::setPorts( Config &config )
{
	std::vector<Server>&	allServers = config.getAllServers();
	std::vector<int>&		allPorts = config.getListen();

	for (size_t i = 0; i < allPorts.size(); i++) {
		// if the port has not been set in _ports add it to the vector
		if (std::find(_ports.begin(), _ports.end(), allPorts[i]) == _ports.end())
			_ports.push_back(allPorts[i]);
	}

	for (size_t i = 0; i < allServers.size(); i++) {
		std::vector<int>	serverPorts = allServers[i].getListen();
		
		for (size_t j = 0; j < serverPorts.size(); j++) {
			// if the port has not been set in _ports add it to the vector
			if (std::find(_ports.begin(), _ports.end(), serverPorts[j]) == _ports.end())
				_ports.push_back(serverPorts[j]);
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
void	HttpServer::createSockets( void )
{
	int	reuse = 1;

	for (size_t i = 0; i < _ports.size(); i++)
	{
		/* Create socket. The socket function returns an integer that is 
		used as a file descriptor. AF_INET = IPv4, SOCK_STREAM = TCP, 
		0 = default protocol */
		int	newSocket = socket( AF_INET, SOCK_STREAM, 0 );

		/* Check if socket creation was successful. If not, throw error */
		if ( newSocket < 0 ) {
			closeServerSockets();
			throw ( std::runtime_error( "Failed to create socket" ) );
		}

		/* Allow reuse of a socket if it has recently been in use */
		if (setsockopt(newSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0) {
			close(newSocket);
			closeServerSockets();
    		throw std::runtime_error( "Failed to set socket options" );
		}

		/* Sets the socket to non-blocking */
		if (fcntl(newSocket, F_SETFL, O_NONBLOCK) < 0) {
			close(newSocket);
			closeServerSockets();
    		throw std::runtime_error( "Failed to set socket to non-blocking" );
		}

		_serverSockets.push_back(newSocket);
	}
}

/**
 * @brief Bind the socket to the port specified in the config
 * @param config The config object containing all the server information
 * @throw std::runtime_error if something goes wrong during the binding
*/
void	HttpServer::bindSockets( void )
{
	sockaddr_in					address;

	for (size_t i = 0; i < _ports.size(); i++) {
		address.sin_family = AF_INET; // IPv4
		address.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
		address.sin_port = htons( _ports[i] ); // Port to listen on

		/* Bind socket to port. If it fails, throw an error */
		if ( bind( _serverSockets[i], ( struct sockaddr * )&address, sizeof( address ) ) < 0 ) {
			closeServerSockets();
			throw ( std::runtime_error( "Failed to bind socket to port" ) );
		}
	}
}

void	HttpServer::listenToSockets( void )
{
	/* Start listening. MAX_CONNECTIONS is defined in HttpServer.hpp */
	for (size_t i = 0; i < _serverSockets.size(); i++) {
		if ( listen( _serverSockets[i], MAX_CONNECTIONS ) < 0) {
			closeServerSockets();
			throw ( std::runtime_error( "Failed to start listening" ));
		}
	}
}

void	HttpServer::pollSockets( void )
{	
	// Add all the sockets to our poll list
	for (size_t i = 0; i < _serverSockets.size(); i++) {
		_poll.addEvent(_serverSockets[i], POLLIN);
	}
}
