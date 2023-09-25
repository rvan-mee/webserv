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
// #include <KqueueUtils.hpp>
#include <sys/socket.h>
#include <sys/event.h>
#include <unistd.h>
#include <stdlib.h>
#include <HttpRequest.hpp>

#include <iostream>

#define READ_SIZE 1024 * 1024
#define WRITE_SIZE 1024 * 1024

#define READ 0
#define WRITE 1

#define RESET   "\033[0m"
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define BLUE    "\033[34m"      /* Blue */

ClientHandler::ClientHandler( int socketFd, EventPoll& poll, Config& config ) :
	_socketFd(socketFd),
	_cgi(poll),
	_bytesWritten(0),
	_config(config),
	_poll(poll)
{
	this->resetState();
}

ClientHandler::~ClientHandler()
{
	// TODO: remove all poll events related to this?
}

bool	ClientHandler::isEvent( int fd )
{
	return (fd == _socketFd || _cgi.isEvent(fd));
}

static bool	checkIfHeadersAreRead( t_requestData& requestData )
{
	const std::vector<char>		endOfHeaders = {'\r','\n','\r','\n'};
	std::vector<char>&			buffer = requestData.buffer;

	// if the iterator is not set to the end of buffer then we have found our substring
	auto it = std::search(buffer.begin(), buffer.end(), endOfHeaders.begin(), endOfHeaders.end());
	if (it != buffer.end()) {
		requestData.readHeaders = true;
		requestData.headerSize = it - buffer.begin() + endOfHeaders.size();
		return (true);
	}
	return (false);
}

static void	setContentLength( t_requestData& requestData )
{
	std::vector<char>&	buffer = requestData.buffer;
	const std::string	cLength = "Content-Length:";

	auto it = std::search(buffer.begin(), buffer.end(), cLength.begin(), cLength.end());

	if (it == buffer.end())
		return ;
	if (it + 16 > buffer.end())
		return ;

	int	contentLengthIndex = it - buffer.begin() + cLength.length();
	requestData.contentLength = atoi(&buffer[contentLengthIndex]);
	requestData.contentLengthSet = true;
}

static void	moveHeadersIntoBuffer( t_requestData& requestData )
{
	std::vector<char>&		buffer = requestData.buffer;
	std::vector<char>&		chunkedBuffer = requestData.chunkedBuffer;
	const std::vector<char>	endOfHeaders = {'\r','\n','\r','\n'};

	auto	headersIterator = std::search(buffer.begin(), buffer.end(), endOfHeaders.begin(), endOfHeaders.end());
	int		headersEndIndex = headersIterator - buffer.begin() + endOfHeaders.size();

	chunkedBuffer.insert(chunkedBuffer.end(), buffer.begin(), buffer.begin() + headersEndIndex);
	buffer.erase(buffer.begin(), buffer.begin() + headersEndIndex);
}

static void	checkChunkedEncoding( t_requestData& requestData )
{
	std::vector<char>&	buffer = requestData.buffer;
	std::string			transferEncoding = "Transfer-Encoding: ";

	auto it = std::search(buffer.begin(), buffer.end(), transferEncoding.begin(), transferEncoding.end());

	if (it == buffer.end())
		return ;
	if (it + transferEncoding.length() > buffer.end())
		return ;

	int	transferEncodingIndex = it - buffer.begin() + transferEncoding.length();

	// Found a Transfer-Encoding header, checking if the encoding is chunked or not.
	// If the encoding is not chunked we don't support it
	if (std::strncmp(&buffer[transferEncodingIndex], "chunked\r\n", 9) != 0)
		throw ( std::runtime_error("Unsupported encoding type") );

	requestData.chunkedEncoded = true;
}

static bool	setNewChunkSize( t_requestData& requestData )
{
	std::vector<char>&		buffer = requestData.buffer;
	std::string				hexString = "0123456789abcdef";
	size_t					lineBreakLength = 2; // "\r\n"
	size_t					hexStringIndex = 0;
	size_t					i = 0;

	requestData.chunkSize = 0;
	// Simple hex to decimal converter:	
	while (i < buffer.size() && buffer[i] != '\r') {
		hexStringIndex = hexString.find(tolower(buffer[i]));
		if (hexStringIndex != std::string::npos) {
			requestData.chunkSize *= 16;
			requestData.chunkSize += hexStringIndex;
		}
		else
			throw ( std::runtime_error("Bad Request\nInvalid chunk header") );
		i++;
	}

	// If the index of our buffer is not an '\r' or we are at the end of the buffer it is an invalid chunk header
	if (i == buffer.size() || buffer[i] != '\r')
		throw ( std::runtime_error("Bad Request\nInvalid chunk header") );
	buffer.erase(buffer.begin(), buffer.begin() + i + lineBreakLength);

	// The final delimiter chunk will have a size of 0 notifying us that we have read all data
	if (requestData.chunkSize == 0) {
		buffer.clear();
		return ( true );
	}
	return ( false );
}

static void	moveChunk( t_requestData& requestData )
{
	std::vector<char>&		chunkedBuffer = requestData.chunkedBuffer;
	std::vector<char>&		buffer = requestData.buffer;
	size_t					lineBreakLength = 2; // "/r/n"
	size_t					amountToMove;

	amountToMove = requestData.chunkSize;
	if (amountToMove > buffer.size())
		amountToMove = buffer.size();

	chunkedBuffer.insert(chunkedBuffer.end(), buffer.begin(), buffer.begin() + amountToMove);
	requestData.chunkSize -= amountToMove;

	// If we have a chunkSize of 0 we have read the entire chunk and we need to trim the \r\n from the buffer
	if (requestData.chunkSize == 0)
		buffer.erase(buffer.begin(), buffer.begin() + amountToMove + lineBreakLength);
	else
		buffer.erase(buffer.begin(), buffer.begin() + amountToMove);
}

static bool	parseChunkEncoding( t_requestData& requestData )
{
	std::vector<char>&		chunkedBuffer = requestData.chunkedBuffer;
	std::vector<char>&		buffer = requestData.buffer;
	bool					allChunksRead = false;

	if (requestData.movedHeaders == false) {
		moveHeadersIntoBuffer(requestData);
		requestData.movedHeaders = true;
		// return false;
	}

	while (buffer.size() > 0 && allChunksRead == false) {
		if (requestData.chunkSize != 0) // we still have to move stuff out from the old buffer
			moveChunk(requestData);
		else // We need to parse a new chunk size
			allChunksRead = setNewChunkSize(requestData);
	}

	if (allChunksRead) {
		buffer.insert(buffer.end(), chunkedBuffer.begin(), chunkedBuffer.end());
		chunkedBuffer.clear();
		return ( true );
	}
	return ( false );
}

static bool	allRequestDataRead( t_requestData& requestData )
{
	if (requestData.readHeaders == false) {
		if (!checkIfHeadersAreRead(requestData))
			return (false);
		checkChunkedEncoding(requestData);
		setContentLength(requestData);
	}

	if (requestData.contentLengthSet == false && requestData.chunkedEncoded == false)
		return (true);

	if (requestData.chunkedEncoded)
		return ( parseChunkEncoding(requestData) );
	
	// if the request is not chunk encoded we check if we have read everything according to the set content-length
	return (requestData.totalBytesRead - requestData.headerSize >= requestData.contentLength);
}

static void	readFromSocket( int socketFd, t_requestData& requestData )
{
	std::vector<char>&	buffer = requestData.buffer;
	std::vector<char>	newRead(READ_SIZE);
	ssize_t				bytesRead;

	bytesRead = recv( socketFd, newRead.data(), READ_SIZE, 0 );
	if (bytesRead < 0)
		throw ( std::runtime_error("Failed to read from socket") );

	buffer.insert(buffer.end(), newRead.begin(), newRead.begin() + bytesRead);
	requestData.totalBytesRead += bytesRead;
}

void	ClientHandler::resetState( void )
{
	_requestData.buffer.clear();
	_requestData.chunkedBuffer.clear();
	_requestData.contentLength = 0;
	_requestData.contentLengthSet = false;
	_requestData.headerSize = 0;
	_requestData.readHeaders = false;
	_requestData.totalBytesRead = 0;
	_requestData.chunkSize = 0;
	_requestData.movedHeaders = false;
}

void	ClientHandler::handleRead( int fd )
{
	if (_cgi.isEvent(fd)) {
		_cgi.handleRead();
		return ;
	}
	
	readFromSocket(_socketFd, _requestData);

	// if all of the data we expect has not been read yet we add another event filter
	// and wait more more data to be available for reading
	if (!allRequestDataRead(_requestData)) {
		// addKqueueEventFilter(_kqueueFd, _socketFd, EVFILT_READ);
		return ;
	}

	std::cout << GREEN "Received all data" RESET << std::endl;
	// all data has been read, now we can parse and prepare a response
	HttpRequest server;

	// the parseRequest should decide if we enter a CGI or not
	// Go into CGI or create a response
	_response = server.parseRequestAndGiveResponse(_requestData.buffer);
	_poll.addEvent(_socketFd, POLLOUT);
	// addKqueueEventFilter(_kqueueFd, _socketFd, EVFILT_WRITE);
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
		// addKqueueEventFilter(_kqueueFd, _socketFd, EVFILT_WRITE);
		return ;
	}

	std::cout << RED "Sent all data" RESET << std::endl;
	this->resetState();
	_poll.removeEvent(_socketFd, POLLOUT);
}
