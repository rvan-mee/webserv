/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   EventHandler.cpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: rvan-mee <rvan-mee@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/08/21 13:19:21 by rvan-mee      #+#    #+#                 */
/*   Updated: 2023/08/21 20:00:14 by rvan-mee      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <EventHandler.hpp>
#include <sys/socket.h>
#include <unistd.h>

#define READ_SIZE 1024
#define WRITE_SIZE 1024

#define READ 0
#define WRITE 1

EventHandler::EventHandler( int socketFd, int kqueueFd ) : 
	_kqueueFd(kqueueFd),
	_socketFd(socketFd),
	_cgi(kqueueFd),
	_bytesWritten(0)
{
}

EventHandler::~EventHandler()
{
	int	cgiPipeRead;
	int	cgiPipeWrite;

	cgiPipeRead = _cgi.getPipeReadFd();
	cgiPipeWrite = _cgi.getPipeWriteFd();
	if (_socketFd != -1)
		close(_socketFd);
	// TODO: remove all kqueue events related to this 
}

bool	EventHandler::isEvent( int fd )
{
	return (fd == _socketFd || _cgi.isEvent(fd));
}

#include <iostream>

void	EventHandler::handleRead( int fd )
{
	std::vector<char>	newRead(READ_SIZE);
	ssize_t				bytesRead;

	if (fd != _socketFd)
		_cgi.handleRead();
	else {
		bytesRead = recv( this->_socketFd, newRead.data(), READ_SIZE, 0 );
		if (bytesRead < 0) {
			throw ( std::runtime_error("Failed to read from socket") );
		}
		_socketReadBuffer.insert(_socketReadBuffer.end(), newRead.begin(), newRead.end());
		std::cout << _socketReadBuffer.data() << std::endl;

		// TODO: check for EOF and parse data & prepare response or start CGI
	}
}

void	EventHandler::handleWrite( int fd )
{
	int	bytesWrote;

	if (fd != _socketFd)
		_cgi.handleWrite();
	else {
		// TODO: send data back to socket
	}
}

