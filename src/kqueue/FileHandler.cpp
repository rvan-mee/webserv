/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   FileHandler.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: rvan-mee <rvan-mee@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/08/27 14:06:27 by rvan-mee      #+#    #+#                 */
/*   Updated: 2023/08/30 12:41:28 by rvan-mee      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <FileHandler.hpp>
#include <KqueueUtils.hpp>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define READ_SIZE 1024
#define WRITE_SIZE 1024

/*	Constructor & Deconstructor */

FileHandler::FileHandler( int kqueueFd ) :
	_fd(-1),
	_kqueueFd(kqueueFd),
	_fileSize(-1),
	_bytesRead(0)
{
}

FileHandler::~FileHandler()
{
}

/*			File Functions					*/

bool	checkFilePermissions( std::string& fileName, int mode )
{
	if (access(fileName.c_str(), mode) == -1)
		return ( false );
	return ( true );
}


bool	fileExists( std::string& fileName )
{
	if (access(fileName.c_str(), F_OK) == -1)
		return ( false );
	return ( true );
}


int	getFileSize( int fd )
{
	struct stat st;

	fstat(fd, &st);
	return ( st.st_size );
}


void	FileHandler::openFile( std::string& fileName, int mode )
{
	if (checkFilePermissions(fileName, mode) == false)
		throw ( std::runtime_error("File does not the the right permissions") );
	
	_fd = open(fileName.c_str(), mode);
	if (_fd == -1)
		throw ( std::runtime_error("Failed to open file") );
	
	if (fcntl(_fd, F_SETFL, O_NONBLOCK) < 0)
		throw ( std::runtime_error("Failed to set the file to non-blocking") );

	_fileSize = getFileSize(_fd);
}


void	FileHandler::createFile( std::string& fileName, std::string& writeBuffer )
{
	if (fileExists(fileName))
		throw ( std::runtime_error("File already exists") );

	_fd = open(fileName.c_str(), O_CREAT | O_RDWR, 0644);
	if (_fd == -1)
		throw ( std::runtime_error("Failed to create a file") );

	if (fcntl(_fd, F_SETFL, O_NONBLOCK) < 0)
		throw ( std::runtime_error("Failed to set the file to non-blocking") );

	_writeBuffer = writeBuffer;

	addKqueueEventFilter(_kqueueFd, _fd, EVFILT_WRITE);
}


/*				getters & setters					*/

bool	FileHandler::isEvent( int fd )
{
	return ( fd == _fd );
}


std::vector<char>&	FileHandler::getReadBuffer( void )
{
	return ( _readBuffer );
}


/*				I/O Operations						*/

t_fileState	FileHandler::handleRead( void )
{
	std::vector<char>	newRead(READ_SIZE);
	ssize_t				bytesRead;

	bytesRead = read(_fd, newRead.data(), READ_SIZE);
	if (bytesRead < 0)
		throw ( std::runtime_error("Failed to read from socket") );

	_readBuffer.insert(_readBuffer.end(), newRead.begin(), newRead.end());
	_bytesRead += bytesRead;

	if (_bytesRead == _fileSize)
		return ( DONE );

	addKqueueEventFilter(_kqueueFd, _fd, EVFILT_READ);
	return ( BUSY );
}


t_fileState	FileHandler::handleWrite( void )
{
	size_t	bytesToWrite = WRITE_SIZE;
	if (bytesToWrite > _writeBuffer.size())
		bytesToWrite = _writeBuffer.size();

	// Write from the buffer into the file
	ssize_t bytesWritten = write(_fd, _writeBuffer.data(), bytesToWrite);
	if (bytesWritten < 0)
		throw ( std::runtime_error("Failed to send response to client") );

	_writeBuffer.erase(0, bytesWritten);

	// if all data hasn't been written yet:
	if (_writeBuffer.size() != 0) {
		addKqueueEventFilter(_kqueueFd, _fd, EVFILT_WRITE);
		return ( BUSY );
	}

	close(_fd);
	return ( DONE );
}


