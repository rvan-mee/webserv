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
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>

#define READ_SIZE 1024 * 1024
// #define WRITE_SIZE 1024 * 16 // Slow write to show that the server is non-blocking
#define WRITE_SIZE 1024 * 1024 * 16

#define READ 0
#define WRITE 1

#define RESET   "\033[0m"
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define BLUE    "\033[34m"      /* Blue */

typedef enum	e_readState {
	DONE_READING,
	BUSY_READING,
	BODY_TOO_LARGE,
}				t_readState;

ClientHandler::ClientHandler( int socketFd, EventPoll& poll, Config& config, int port, char* clientAddress ) :
	_socketFd(socketFd),
	_port(port),
	_config(config),
	_poll(poll),
	_doneReading(false),
	_doneWriting(true),
	_pollHupSet(false),
	_timeOutSet(false),
	_terminateAfterResponse(false),
	_clientAddress(clientAddress),
	_cgi(poll, config, port, _clientAddress),
	_request(_cgi, _poll, _socketFd)
{
	this->clear();
	this->resetTimeOut();
}

ClientHandler::~ClientHandler()
{
}

static std::string	getHost( std::vector<char>& buffer)
{
	char const*	host = "Host";
	std::string	value;

	// find the header
	auto it = std::search(buffer.begin(), buffer.end(), host, host + strlen(host));
	// skip till ":"
	for (; it != buffer.end(); it++)
		if (*it == ':')
			break ;
	// skip ": "
	if (it == buffer.end() || ++it == buffer.end())
		return ("");
	if (++it == buffer.end())
		return ("");
	// add to the value
	for (; it != buffer.end() && *it != '\r'; it++) {
		if (*it == ':') // todo: is this necessary? what if there is no port in the host header?
			break ;
		value.push_back(*it);
	}
	return (value);
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
		requestData.host = getHost(buffer);
		if (requestData.host == "")
			requestData.host = "_";
		return (true);
	}
	return (false);
}

/**
 * @return false if content length is too big else true
*/
static bool	setContentLength( t_requestData& requestData, Config& config )
{
	std::vector<char>&	buffer = requestData.buffer;
	const std::string	cLength = "Content-Length:";

	auto it = std::search(buffer.begin(), buffer.end(), cLength.begin(), cLength.end());

	if (it == buffer.end())
		return (true);
	if (it + 16 > buffer.end())
		return (true);

	int	contentLengthIndex = it - buffer.begin() + cLength.length();
	requestData.contentLength = atoi(&buffer[contentLengthIndex]);
	requestData.contentLengthSet = true;
	if (requestData.contentLength > config.getClientMaxBodySize())
		return (false);
	return (true);
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

static t_readState	parseChunkEncoding( t_requestData& requestData )
{
	std::vector<char>&		chunkedBuffer = requestData.chunkedBuffer;
	std::vector<char>&		buffer = requestData.buffer;
	bool					allChunksRead = false;

	if (requestData.movedHeaders == false) {
		moveHeadersIntoBuffer(requestData);
		requestData.movedHeaders = true;
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
		return ( DONE_READING );
	}
	return ( BUSY_READING );
}

static t_readState	allRequestDataRead( t_requestData& requestData, Config& config )
{
	if (requestData.readHeaders == false) {
		if (checkIfHeadersAreRead(requestData) == false)
			return (BUSY_READING);
		checkChunkedEncoding(requestData);
		if (setContentLength(requestData, config) == false)
			return (BODY_TOO_LARGE);
	}

	if (requestData.contentLengthSet == false && requestData.chunkedEncoded == false)
		return (DONE_READING);

	if (requestData.chunkedEncoded)
		return ( parseChunkEncoding(requestData) );
	
	// if the request is not chunk encoded we check if we have read everything according to the set content-length
	if (!(requestData.totalBytesRead - requestData.headerSize >= requestData.contentLength))
		return (BUSY_READING); // We have not read everything from this request

	// We have read everything from the current request.
	// Store the entire request inside the buffer and move all the other data into prevBuffer
	auto extraDataIt = requestData.buffer.begin() + requestData.headerSize + requestData.contentLength; // Gets the offset where the data past the request is stored

	requestData.prevBuffer.clear();
	if (requestData.buffer.size() - (requestData.headerSize + requestData.contentLength) > 0) {
		requestData.prevBuffer.insert(requestData.prevBuffer.begin(), extraDataIt, requestData.buffer.end()); //  Insert all the extra data into prevBuffer
		requestData.buffer.erase(extraDataIt); // Remove the extra data from the buffer
	}
	return (DONE_READING);
}

void	ClientHandler::readFromSocket()
{
	std::vector<char>&	buffer = _requestData.buffer;
	std::vector<char>	newRead(READ_SIZE);
	ssize_t				bytesRead;

	bytesRead = recv( _socketFd, newRead.data(), READ_SIZE, 0 );
	if (bytesRead < 0)
		throw ( std::runtime_error("Failed to read from socket") );

	if (_pollHupSet && bytesRead == 0) {
		_doneReading = true;
		return ;
	}

	buffer.insert(buffer.end(), newRead.begin(), newRead.begin() + bytesRead);
	_requestData.totalBytesRead += bytesRead;
}

void	ClientHandler::setHup( void )
{
	if (!_pollHupSet) {
		std::cout << RED "Client hang-up" RESET "\n";
		_timeOutSet = true;
		_pollHupSet = true;
		this->resetTimeOut();
	}
}

void	ClientHandler::setTimeOutResponse( bool cgiRunning )
{
	HttpResponse	timeOutResponse;

	if (cgiRunning)
		timeOutResponse.setError(502, "502.1 CGI application timeout");
	else
		timeOutResponse.setError(408, "Request Timeout");
	_response = timeOutResponse.buildResponse(_config.getServer(_requestData.host, _port));
	_doneWriting = false;
}

void	ClientHandler::prepareNextRequest( void )
{
	_cgi.clear();
	_response.clear();
	_requestData.buffer.clear();
	_requestData.chunkedBuffer.clear();
	_requestData.chunkSize = 0;
	_requestData.movedHeaders = false;
	_requestData.readHeaders = false;
	_requestData.contentLengthSet = false;
	_requestData.chunkedEncoded = false;
	_requestData.headerSize = 0;
	_requestData.contentLength = 0;
	_requestData.totalBytesRead = _requestData.prevBuffer.size();
	_requestData.host.clear();
	_doneWriting = true;
	_doneReading = false;
}

void	ClientHandler::clear( void )
{
	_cgi.clear();
	_response.clear();
	_requestData.buffer.clear();
	_requestData.chunkedBuffer.clear();
	_requestData.prevBuffer.clear();
	_requestData.chunkSize = 0;
	_requestData.movedHeaders = false;
	_requestData.readHeaders = false;
	_requestData.contentLengthSet = false;
	_requestData.chunkedEncoded = false;
	_requestData.headerSize = 0;
	_requestData.contentLength = 0;
	_requestData.totalBytesRead = 0;
	_requestData.host.clear();
	_doneWriting = true;
	_doneReading = false;
}

void	ClientHandler::endCgi()
{
	_cgi.end();
	_response = std::string(_cgi.getReadBuffer().data());
	_cgi.clear();
	_poll.addEvent(_socketFd, POLLOUT);
}

void	ClientHandler::resetTimeOut( void )
{
	_timeOutStart = std::chrono::steady_clock::now();
}

/**
 * @brief	Checks if this client has been timed out on the CGI or socket
 * 			and sets a response message accordingly
 * 
 * @param currentTime The time to compare the last interaction to
 * @return [true] if the client timed out  ---  
 * @return [false] if the client has not timed out yet
 */
bool	ClientHandler::checkTimeOut( serverTime& currentTime )
{
	int64_t	timeDifference = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - _timeOutStart).count();

	if ( timeDifference >= REQUEST_TIMEOUT) {
		std::cout << RED "Request or connection timed out" RESET "\n";
		bool	cgiRunning = this->isCgiRunning();
		this->clear();
		_timeOutSet = true;
		_doneReading = true;
		_doneWriting = true;
		this->setTimeOutResponse(cgiRunning);
		_poll.removeEvent(_socketFd, POLLIN | POLLRDHUP);
		_poll.removeEvent(_socketFd, POLLOUT);
		_poll.addEvent(_socketFd, POLLOUT );
		this->resetTimeOut();
		return (true);
	}
	return (false);
}

void	ClientHandler::handleRead( int fd )
{

	if (_cgi.isEvent(fd)) {
		this->resetTimeOut();
		_cgi.handleRead();
		return ;
	}

	// If we are already processing a request we keep the new data in the socket
	// and don't read from it.
	if (!_doneWriting || _cgi.isRunning())
		return ;

	this->resetTimeOut();
	this->readFromSocket();

	// if all of the data we expect has not been read yet we add another event filter
	// and wait more more data to be available for reading
	t_readState	state;

	state = allRequestDataRead(_requestData, _config);
	if (state == BUSY_READING)
		return;
	else if (state == BODY_TOO_LARGE) {
		HttpResponse	bodyTooLarge;

		bodyTooLarge.setError(413, "Content Too Large");
		_response = bodyTooLarge.buildResponse(_config.getServer(_requestData.host, _port));
		_poll.addEvent(_socketFd, POLLOUT);
		_terminateAfterResponse = true;
		_doneWriting = false;
		return ;
	}

	std::cout << GREEN "Received all data" RESET "\n";
	// all data has been read, now we can parse and prepare a response
	_request.setContentLength(_requestData.buffer.size() - _requestData.headerSize);
	_response = _request.parseRequestAndGiveResponse(_requestData.buffer, _config, _port);
	_doneWriting = false;
}

void	ClientHandler::handleWrite( int fd )
{
	this->resetTimeOut();

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
		return ;
	}

	std::cout << GREEN "Sent all data" RESET "\n";

	if (_timeOutSet) {
		_doneWriting = true;
		return ;
	}

	// We have sent all data so we can prepare for the next request
	this->prepareNextRequest();

	// We move all the data that we might've read extra the previous time into the current buffer
	_requestData.buffer.clear();
	_requestData.buffer = _requestData.prevBuffer;

	// We check if we already have fully read a new request we can keep the write event inside the poll check
	// otherwise we need to remove it from the queue
	t_readState	state = allRequestDataRead(_requestData, _config);
	if (state == DONE_READING)
		_doneWriting = false;
	else if (state == BUSY_READING)
		_poll.removeEvent(_socketFd, POLLOUT);
	else if (state == BODY_TOO_LARGE) {
		HttpResponse	bodyTooLarge;

		bodyTooLarge.setError(413, "Content Too Large");
		_response = bodyTooLarge.buildResponse(_config.getServer(_requestData.host, _port));
		_terminateAfterResponse = true;
		_doneWriting = true;
	}
}
