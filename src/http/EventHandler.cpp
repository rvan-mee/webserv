/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   EventHandler.cpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: rvan-mee <rvan-mee@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/08/21 13:19:21 by rvan-mee      #+#    #+#                 */
/*   Updated: 2023/08/23 19:55:58 by rvan-mee      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <EventHandler.hpp>
#include <sys/socket.h>
#include <sys/event.h>
#include <unistd.h>

#define READ_SIZE 10
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
	// TODO: remove all kqueue events related to this?

	// it seems events get automatically deleted on FD closure.
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

	if (fd != _socketFd) {
		_cgi.handleRead();
		return ;
	}

	bytesRead = recv( this->_socketFd, newRead.data(), READ_SIZE, 0 );
	if (bytesRead < 0) {
		throw ( std::runtime_error("Failed to read from socket") );
	}
	_socketReadBuffer.insert(_socketReadBuffer.end(), newRead.begin(), newRead.end());
	std::cout << "total bytes read: " << _socketReadBuffer.size() << std::endl;
	std::cout << "bytes read: " << bytesRead << std::endl << std::endl;

	struct kevent	evSet;
	// Adding the read event back to kqueue, using EV_CLEAR it removes it after it has been handled.
	EV_SET( &evSet, fd, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, NULL );
	if ( kevent( _kqueueFd, &evSet, 1, NULL, 0, NULL ) < 0 )
		throw ( std::runtime_error( "Failed to add a read event to kqueue" ) );
	// TODO: check for /r/n/r/n to get to the end of the headers.
	// After that check if content-length is defined and read till we reached that length.
}

void	EventHandler::handleWrite( int fd )
{
	int	bytesWrote;

	if (fd != _socketFd) {
		_cgi.handleWrite();
		return ;
	}

	// TODO: send data back to socket

	// if all data hasn't been sent yet:
	struct kevent	evSet;
	EV_SET( &evSet, fd, EVFILT_WRITE, EV_ADD | EV_CLEAR, 0, 0, NULL );
	if ( kevent( _kqueueFd, &evSet, 1, NULL, 0, NULL ) < 0 )
		throw ( std::runtime_error( "Failed to add a write event to kqueue" ) );
}
