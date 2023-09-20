/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   CgiHandler.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: rvan-mee <rvan-mee@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/08/21 14:48:22 by rvan-mee      #+#    #+#                 */
/*   Updated: 2023/09/20 21:03:04 by rvan-mee      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <CgiHandler.hpp>
#include <unistd.h>
#include <signal.h>

#define WRITE_SIZE 1024
#define READ_SIZE 1024

CgiHandler::CgiHandler( EventPoll& poll ) :
	_poll(poll),
	_pipeRead(-1),
	_pipeWrite(-1),
	_bytesRead(0),
	_bytesWrote(0),
	_forkPid(-1)
{
	_cgiOutput.reserve(WRITE_SIZE);
}

CgiHandler::~CgiHandler()
{
	if (_pipeRead != -1)
		close(_pipeRead);
	if (_pipeWrite != -1)
		close(_pipeWrite);	
	if (_forkPid != -1 && waitpid(_forkPid, NULL, WNOHANG) == 0) // TODO: does the waitpid work?? gotta test
		kill(_forkPid, SIGKILL);
}

void	CgiHandler::setWriteBuffer( std::vector<char>& buffer )
{
	_cgiInput = buffer;
}

bool	CgiHandler::isEvent(int fd)
{
	return (fd == _pipeRead || fd == _pipeWrite);
}

//	************************ I/O Handlers ************************

void	CgiHandler::handleRead( void )
{
	int	currentBytesRead;

	if (_cgiOutput.size() < _cgiOutput.capacity() + READ_SIZE)
		_cgiOutput.resize(_cgiOutput.size() * 2);
	currentBytesRead = read(_pipeRead, &(_cgiOutput[_bytesRead]), READ_SIZE);
	if (currentBytesRead < 0)
		throw ( std::runtime_error("Failed to read from the CGI") );
	_bytesRead += currentBytesRead;
	
	// if
	// TODO: check for EOF
	// get output into _socketBuffer from EventHandler?
	// _cgiOutput.shrink_to_fit();
	// else
	// TODO: create new read event in kqueue
}

void	CgiHandler::handleWrite( void )
{
	const int	bytesLeft = _cgiInput.size() - _bytesWrote;
	int			bytesToWrite = WRITE_SIZE;
	int			currentWriteAmount;

	if (bytesLeft < WRITE_SIZE)
		bytesToWrite = bytesLeft;

	currentWriteAmount = write(_pipeWrite, &(_cgiInput[_bytesWrote]), bytesToWrite);
	if (currentWriteAmount < 0)
		throw ( std::runtime_error("Failed to write to the CGI") );
	_bytesWrote += currentWriteAmount;

	// TODO: create new write event in the kqueue
}
