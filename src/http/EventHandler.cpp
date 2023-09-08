/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   EventHandler.cpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: rvan-mee <rvan-mee@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/08/21 13:19:21 by rvan-mee      #+#    #+#                 */
/*   Updated: 2023/08/25 13:41:16 by dkramer       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <EventHandler.hpp>
#include <KqueueUtils.hpp>
#include <sys/socket.h>
#include <sys/event.h>
#include <unistd.h>
#include <stdlib.h>
#include <HttpRequest.hpp>

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
	_requestData.totalBytesRead = 0;
	_requestData.headerSize = 0;
	_requestData.contentLength = 0;
	_requestData.readHeaders = false;
	_requestData.contentLengthSet = false;
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

static bool	checkIfHeadersAreRead( t_requestData& requestData )
{
	std::vector<char>::iterator	it;
	const std::vector<char>		endOfHeaders = {'\r','\n','\r','\n'};
	std::vector<char>&			buffer = requestData.buffer;

	// if the iterator is not set to the end of buffer then we have found our substring
	it = std::search(buffer.begin(), buffer.end(), endOfHeaders.begin(), endOfHeaders.end());
	if (it != buffer.end()) {
		requestData.readHeaders = true;
		requestData.headerSize = it - buffer.begin() + 4;
		return (true);
	}
	return (false);
}

static void	setContentLength( t_requestData& requestData )
{
	std::string	headers(requestData.buffer.data());
	size_t		pos;

	pos = headers.find("Content-Length");
	if (pos == std::string::npos)
		return ;
	if (pos + 16 > headers.length())
		return ;
	requestData.contentLength = atoi(&headers[pos + 16]);
	std::cout << "Content-Length: " << requestData.contentLength << std::endl;
	requestData.contentLengthSet = true;
}

static bool	allRequestDataRead( t_requestData& requestData )
{
	if (requestData.readHeaders == false) {
		if (checkIfHeadersAreRead(requestData) == false)
			return (false);
		setContentLength(requestData);
	}

	if (requestData.contentLengthSet == false) {
		return (true);
	}

	if (requestData.totalBytesRead - requestData.headerSize == requestData.contentLength)
		return (true);
	return (false);
}

static void	readIntoBuffer( int socketFd, t_requestData& requestData )
{
	std::vector<char>&	buffer = requestData.buffer;
	std::vector<char>	newRead(READ_SIZE);
	int					bytesRead;

	bytesRead = recv( socketFd, newRead.data(), READ_SIZE, 0 );
	if (bytesRead < 0)
		throw ( std::runtime_error("Failed to read from socket") );

	buffer.insert(buffer.end(), newRead.begin(), newRead.end());
	requestData.totalBytesRead += bytesRead;
}

void	EventHandler::handleRead( int fd, Config &config )
{
	if (fd != _socketFd) {
		_cgi.handleRead();
		return ;
	}

	readIntoBuffer(_socketFd, _requestData );
	// if all of the data we expect has not been read yet we add another event filter
	// and wait more more data to be available for reading
	if (!allRequestDataRead(_requestData)) {
		addKqueueEventFilter(_kqueueFd, _socketFd, EVFILT_READ);
		return ;
	}

	// all data has been read, now we can parse and prepare a response
	std::cout << "Request: " << _requestData.buffer.data() << std::endl;
	// Start parsing the request data
	HttpRequest server;
	
	// std::vector<char> v;
	// std::copy(s.begin(), s.end(), std::back_inserter(v));
	std::string response = server.parseRequestandGiveReponse(_requestData.buffer, config);
	
	// Convert the response string to bytes
	const char *responseBytes = response.c_str();
	// size_t responseSize = response.size();
	// Send the response to the client
	ssize_t bytesSent = send(_socketFd, responseBytes,
	(int)strlen(responseBytes), 0);
	if (bytesSent < 0) {
		std::cerr << "Failed to send response to client" << std::endl;
	} else {
		if (fflush(stdout) != 0) {
			std::cerr << "Failed to flush output buffers" << std::endl;
		}
	}
	std::cout << "eventfd: " << _socketFd << " Sent response: " << responseBytes << std::endl;
	// Go into CGI or create a response
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
	addKqueueEventFilter(_kqueueFd, _socketFd, EVFILT_WRITE);
}
