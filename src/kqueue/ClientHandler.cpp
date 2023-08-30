/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ClientHandler.cpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: rvan-mee <rvan-mee@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/08/21 13:19:21 by rvan-mee      #+#    #+#                 */
/*   Updated: 2023/08/28 18:46:58 by rvan-mee      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <ClientHandler.hpp>
#include <KqueueUtils.hpp>
#include <sys/socket.h>
#include <sys/event.h>
#include <unistd.h>
#include <stdlib.h>
#include <HttpRequest.hpp>

#include <iostream>

#define READ_SIZE 1024
#define WRITE_SIZE 1024

#define READ 0
#define WRITE 1

ClientHandler::ClientHandler( int socketFd, int kqueueFd ) : 
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

ClientHandler::~ClientHandler()
{
	// TODO: remove all kqueue events related to this?

	// it seems events get automatically deleted on FD closure.
}

bool	ClientHandler::isEvent( int fd )
{
	return (fd == _socketFd || _cgi.isEvent(fd));
}

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
	std::vector<char>&				buffer = requestData.buffer;
	const char*						cLength = "Content-Length:";
	std::vector<char>::iterator 	it;

	it = std::search(buffer.begin(), buffer.end(), cLength, cLength + 16);

	if (it == buffer.end())
		return ;
	if (it + 16 > buffer.end())
		return ;

	int	contentLengthIndex = it - buffer.begin() + 16;
	requestData.contentLength = atoi(&buffer[contentLengthIndex]);
	requestData.contentLengthSet = true;
}

static bool	allRequestDataRead( t_requestData& requestData )
{
	if (requestData.readHeaders == false) {
		if (!checkIfHeadersAreRead(requestData))
			return (false);
		setContentLength(requestData);
	}

	if (requestData.contentLengthSet == false) {
		return (true);
	}

	if (requestData.totalBytesRead - requestData.headerSize >= requestData.contentLength)
		return (true);
	return (false);
}

static void	readFromSocket( int socketFd, t_requestData& requestData )
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

void	ClientHandler::handleRead( int fd )
{
	if (_cgi.isEvent(fd)) {
		_cgi.handleRead();
		return ;
	}
	// else if () fd is from a fileHandler inside the parseRequest


	readFromSocket(_socketFd, _requestData);
	// if all of the data we expect has not been read yet we add another event filter
	// and wait more more data to be available for reading
	if (!allRequestDataRead(_requestData)) {
		addKqueueEventFilter(_kqueueFd, _socketFd, EVFILT_READ);
		return ;
	}

	std::cout << "Received all data" << std::endl;
	// all data has been read, now we can parse and prepare a response
	HttpRequest server;

	// the parseRequest should decide if we enter a CGI or not
	// Go into CGI or create a response
	_response = server.parseRequestAndGiveResponse(_requestData.buffer);
	addKqueueEventFilter(_kqueueFd, _socketFd, EVFILT_WRITE);
}

void	ClientHandler::handleWrite( int fd )
{
	if (fd != _socketFd) {
		_cgi.handleWrite();
		return ;
	}

	size_t	bytesToWrite = WRITE_SIZE;
	if (bytesToWrite > _response.size())
		bytesToWrite = _response.size();

	// Send the response to the client
	ssize_t bytesSent = send(_socketFd, _response.c_str(), bytesToWrite, 0);
	if (bytesSent < 0)
		throw ( std::runtime_error("Failed to send response to client") );

	_response.erase(0, bytesSent);

	// if all data hasn't been sent yet:
	if (_response.size() != 0) {
		addKqueueEventFilter(_kqueueFd, _socketFd, EVFILT_WRITE);
		return ;
	}

	std::cout << "Sent all data" << std::endl;
}
