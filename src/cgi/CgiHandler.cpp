/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   CgiHandler.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: rvan-mee <rvan-mee@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/08/21 14:48:22 by rvan-mee      #+#    #+#                 */
/*   Updated: 2023/09/25 14:40:20 by dkramer       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <CgiHandler.hpp>
#include <unistd.h>
#include <signal.h>
#include <KqueueUtils.hpp>
#include <iostream>
#include <vector>
#include <sys/event.h>

#define WRITE_SIZE 1024
#define READ_SIZE 1024

CgiHandler::CgiHandler( int kqueueFd ) :
	_kqueueFd(kqueueFd),
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
	int i = 0;
	while (i < _cgiOutput.size())
	{
		std::cout << _cgiOutput[i] << std::endl;
	}
	// if
	// TODO: check for EOF
	// get output into _socketBuffer from EventHandler?
	// _cgiOutput.shrink_to_fit();
	// else
	// TODO: create new read event in kqueue
}

void	CgiHandler::handleWrite( void )
{
	size_t	bytesToWrite = WRITE_SIZE;

	if (bytesToWrite > _cgiInput.size())
		bytesToWrite = _cgiInput.size();

	ssize_t bytesSent = write(_pipeWrite, _cgiInput.data(), bytesToWrite);
	if (bytesSent < 0)
		throw ( std::runtime_error("Failed to send response to client") );

	_cgiInput.erase(_cgiInput.begin(), _cgiInput.begin() + bytesSent);

	if (_cgiInput.size() == 0) {
		// wrote everything to pipe
		addKqueueEventFilter( _kqueueFd, _pipeRead, EVFILT_READ );
		return ;
	}
	addKqueueEventFilter( _kqueueFd, _pipeWrite, EVFILT_WRITE );
}

/**
 * @brief Starts a Python CGI script. 
 * @note 2 pipes are created.
 * In the child process we close the write end pipeToCgi[1] and the read end pipeFromCgi[0].
 * In the parent process we close the read end pipeToCgi[0] and the write end pipeFromCgi[1].
 * pipeToCgi is used to send data to the CGI script (parent to child). 
 * pipeFromCgi is used to receive data from the CGI script (child to parent).
 * Result:
 *  ________                 _______
 * |        |  pipeToCgi    |       |
 * |        |  >--------->  |       |
 * | Parent |               | Child |
 * |        |  pipeFromCgi  |       |
 * |________|  <---------<  |_______|            
 */
void	CgiHandler::startPythonCgi( void )
{
	int pipeToCgi[2];
	int pipeFromCgi[2];
	
	// temp
	char *args[] = { "python", "/Users/cpost/Desktop/webserv/src/test.py", NULL };
	
	// Init pipes. Throws runtime_error on failure.
	if ( pipe( pipeToCgi ) == -1)
		throw ( std::runtime_error("Failed to create pipe") );
	if ( pipe( pipeFromCgi ) == -1 )
	{
		close( pipeToCgi[0] );
		close( pipeToCgi[1] );
		throw ( std::runtime_error("Failed to create pipe") );
	}
		
	// Fork process. Throws runtime_error on failure.
	this->_forkPid = fork();
	if ( this->_forkPid == -1 )
	{
		close( pipeToCgi[0] );
		close( pipeToCgi[1] );
		close( pipeFromCgi[0] );
		close( pipeFromCgi[1] );
		throw ( std::runtime_error( "Failed to fork process" ) );
	}
	else if ( this->_forkPid == 0 ) // Child process
	{
		// Setup pipes in child process
		this->childInitPipes( pipeToCgi, pipeFromCgi );

		// Execute the CGI script
    	char *env[] = { NULL };
		execve( PYTHON_PATH, args, env );
		std::cerr << "Error executing Python script" << std::endl;
		exit( 1 ) ;
	}
	else // Parent process
	{
		// Setup pipes in parent process
		this->parentInitPipes( pipeToCgi, pipeFromCgi );

		addKqueueEventFilter( _kqueueFd, _pipeWrite, EVFILT_WRITE );
	}
}

/**
 * @brief Setup pipes in child process
 * @param pipeToCgi 
 * @param pipeFromCgi 
 */
void	CgiHandler::childInitPipes( int pipeToCgi[2], int pipeFromCgi[2])
{
	// close unused pipe ends
	close( pipeToCgi[1] ); // Close write end of pipeToCgi
	close( pipeFromCgi[0] ); // Close read end of pipeFromCgi

	// Redirect stdin and stdout to the pipes
    dup2( pipeToCgi[0], STDIN_FILENO ); // Redirect pipeToCgi to stdin
    dup2( pipeFromCgi[1], STDOUT_FILENO ); // Redirect pipeFromCgi to stdout

	// Close the remaining pipe ends that are not used
	close( pipeToCgi[0] ); // Close read end of pipeToCgi
	close( pipeFromCgi[1] ); // Close write end of pipeFromCgi
}

/**
 * @brief Setup pipes in parent process
 * @param pipeToCgi 
 * @param pipeFromCgi 
 */
void	CgiHandler::parentInitPipes( int pipeToCgi[2], int pipeFromCgi[2] )
{
	// close unused pipe ends
	close( pipeToCgi[0] ); // Close read end of pipeToCgi
	close( pipeFromCgi[1] ); // Close write end of pipeFromCgi

	// Set the pipe ends to the class variables
	this->_pipeWrite = pipeToCgi[1]; // Write end of pipeToCgi. Send data to CGI script.
	this->_pipeRead = pipeFromCgi[0]; // Read end of pipeFromCgi. Receive data from CGI script.
}
