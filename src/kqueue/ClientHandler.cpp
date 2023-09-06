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

#define READ_SIZE 10240
#define WRITE_SIZE 65536

#define READ 0
#define WRITE 1

#define RESET   "\033[0m"
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define BLUE    "\033[34m"      /* Blue */

ClientHandler::ClientHandler( int socketFd, int kqueueFd, Config& config ) : 
	_kqueueFd(kqueueFd),
	_socketFd(socketFd),
	_cgi(kqueueFd),
	_bytesWritten(0),
	_config(config)
{
	_requestData.totalBytesRead = 0;
	_requestData.headerSize = 0;
	_requestData.contentLength = 0;
	_requestData.chunkSize = 0;
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
	std::cout << "before: " << buffer.data() << std::endl << std::endl << std::endl;
	buffer.erase(buffer.begin(), buffer.begin() + headersEndIndex);
	std::cout << "after: " << buffer.data() << std::endl << std::endl << std::endl;
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
	moveHeadersIntoBuffer(requestData);
	// std::cout << "chunked buffer: " << requestData.chunkedBuffer.data() << std::endl << std::endl << std::endl;
	// std::cout << "regular buffer: " << requestData.buffer.data() << std::endl << std::endl << std::endl;
}

static bool	setNewChunkSize( t_requestData& requestData )
{
	std::vector<char>&		buffer = requestData.buffer;
	size_t					lineBreakLength = 2;
	size_t					indexAfterStoi = 0;

	try {
		// std::cout << "Buffer contains:\n\n" << requestData.buffer.data() << std::endl<< std::endl<< std::endl;
		requestData.chunkSize = std::stoi(buffer.data(), &indexAfterStoi, 16);
		std::cout << "Chunk size: " << requestData.chunkSize << std::endl;
		requestData.buffer.erase(buffer.begin(), buffer.begin() + indexAfterStoi + lineBreakLength * 2);
		if (requestData.chunkSize == 0)
			return ( true );
	}
	catch(const std::exception& e) {
		throw ( std::runtime_error("Invalid block size\nBad Request") );
	}
	return ( false );
}

static void	moveChunk( t_requestData& requestData )
{
	std::vector<char>&		chunkedBuffer = requestData.chunkedBuffer;
	std::vector<char>&		buffer = requestData.buffer;
	size_t					lineBreakLength = 2;
	size_t					amountToMove;

	amountToMove = requestData.chunkSize;
	std::cout << "Amount left from this chunk: " << amountToMove << std::endl;
	if (amountToMove > buffer.size())
		amountToMove = buffer.size();
	std::cout << "After if check: " << amountToMove << std::endl << std::endl << std::endl << std::endl;

	chunkedBuffer.insert(chunkedBuffer.end(), buffer.begin(), buffer.begin() + amountToMove);
	requestData.chunkSize -= amountToMove;

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

	while (buffer.size() > 0 && allChunksRead == false) {
		if (requestData.chunkSize != 0) // we still have to move stuff out from the old buffer
			moveChunk(requestData);
		else // We need to parse a new chunk size
			allChunksRead = setNewChunkSize(requestData);
	}


	if ( allChunksRead ) {
		// Move everything from the chunked buffer into the regular buffer so we can use it in the response parser
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
		addKqueueEventFilter(_kqueueFd, _socketFd, EVFILT_READ);
		return ;
	}

	std::cout << GREEN "Received all data" RESET << std::endl;
	// all data has been read, now we can parse and prepare a response
	HttpRequest server;

	// the parseRequest should decide if we enter a CGI or not
	// Go into CGI or create a response
	_response = server.parseRequestAndGiveResponse(_requestData.buffer);
	addKqueueEventFilter(_kqueueFd, _socketFd, EVFILT_WRITE);
	this->resetState();
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

	std::cout << RED "Sent all data" RESET << std::endl;
}
