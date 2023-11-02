/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   CgiHandler.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: rvan-mee <rvan-mee@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/08/21 14:48:22 by rvan-mee      #+#    #+#                 */
/*   Updated: 2023/09/25 15:05:39 by rvan-mee      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <CgiHandler.hpp>
#include <unistd.h>
#include <signal.h>
#include <iostream>
#include <vector>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

#define WRITE_SIZE 1024
#define READ_SIZE 1024

#define RESET   "\033[0m"
#define GREEN   "\033[32m"      /* Green */

CgiHandler::CgiHandler( EventPoll& poll, Config& config, int port, std::string& clientAddress ) :
	_poll(poll),
	_pipeRead(-1),
	_pipeWrite(-1),
	_bytesRead(0),
	_bytesWrote(0),
	_doneReading(false),
	_port(port),
	_forkPid(-1),
	_config(config),
	_clientAddress(clientAddress)
{
}

CgiHandler::~CgiHandler()
{
	if (_pipeRead != -1)
		close(_pipeRead);
	if (_pipeWrite != -1)
		close(_pipeWrite);
	if (_forkPid != -1 && waitpid(_forkPid, NULL, WNOHANG) == 0) {
		kill(_forkPid, SIGKILL);
	}
	_poll.removeEvent(_pipeWrite, POLLOUT);
	_poll.removeEvent(_pipeRead, POLLIN);
}

static char*	combineEnvironment(char const* key, std::string value)
{
	size_t	len1 = strlen(key);
	size_t	len2 = value.size();
	char*	str = new char[len1 + len2 + 1];

	for (size_t i = 0; i < len1; i++) {
		str[i] = key[i];
	}
	for (size_t i = len1; i < len1 + len2; i++) {
		str[i] = value[i - len1];
	}
	str[len1 + len2] = '\0';
	return (str);
}

std::string	getQueryString(HttpRequest& request)
{
	auto index = request.getUri().find("?");

	if (index == std::string::npos)
		return ("");
	return (request.getUri().substr(index + 1));
}

char**	CgiHandler::getEnvironmentVariables( HttpRequest& request, std::string& script )
{
	char**	env = new char*[15];

	env[0] = combineEnvironment("CONTENT_LENGTH=", std::to_string(request.getContentLength()));
	env[1] = combineEnvironment("CONTENT_TYPE=", request.getContentType());
	env[2] = combineEnvironment("GATEWAY_INTERFACE=CGI/1.1", "");
	env[3] = combineEnvironment("PATH_INFO=", script);
	env[4] = combineEnvironment("PATH_TRANSLATED=", script);
	env[5] = combineEnvironment("QUERY_STRING=", getQueryString(request));
	env[6] = combineEnvironment("REMOTE_ADDR=", _clientAddress);
	env[7] = combineEnvironment("REMOTE_HOST=", request.getHost());
	env[8] = combineEnvironment("REQUEST_METHOD=", (request.getMethod() == GET ? "GET" : "POST"));
	env[9] = combineEnvironment("SCRIPT_NAME=", script);
	env[10] = combineEnvironment("SERVER_NAME=", request.getHost());
	env[11] = combineEnvironment("SERVER_PORT=", std::to_string(_port));
	env[12] = combineEnvironment("SERVER_PROTOCOL=HTTP/1.1", "");
	env[13] = combineEnvironment("SERVER_SOFTWARE=", "Twerkin' Server");
	env[14] = NULL;	
	return (env);
}

void	CgiHandler::clear( void )
{
	if (_pipeRead != -1) {
		_poll.removeEvent(_pipeRead, POLLIN);
		close(_pipeRead);
		_pipeRead = -1;
	}
	if (_pipeWrite != -1) {
		_poll.removeEvent(_pipeWrite, POLLOUT);
		close(_pipeWrite);
		_pipeWrite = -1;
	}
	if (_forkPid != -1 && waitpid(_forkPid, NULL, WNOHANG) == 0) {
		kill(_forkPid, SIGKILL);
	}
	_forkPid = -1;
	_cgiInput.clear();
	_cgiOutput.clear();
	_bytesRead = 0;
	_bytesWrote = 0;
	_doneReading = false;
}

void	CgiHandler::setWriteBuffer( std::vector<char>& buffer )
{
	_cgiInput = buffer;
}

std::vector<char>&	CgiHandler::getReadBuffer( void )
{
	return (_cgiOutput);
}

bool	CgiHandler::isEvent(int fd)
{
	return (fd == _pipeRead || fd == _pipeWrite);
}

bool	CgiHandler::isRunning()
{
	return (_forkPid != -1 && waitpid(_forkPid, NULL, WNOHANG) == 0);
}

bool	CgiHandler::isDoneReading()
{
	return (_doneReading);
}

//	************************ I/O Handlers ************************

// After having written everything into the CGI we can start reading from it
void	CgiHandler::handleRead( void )
{
	std::vector<char>	newRead(READ_SIZE);
	int					currentBytesRead;

	currentBytesRead = read(_pipeRead, newRead.data(), READ_SIZE);
	if (currentBytesRead < 0)
		throw ( std::runtime_error("Failed to read from the CGI") );

	_cgiOutput.insert(_cgiOutput.end(), newRead.begin(), newRead.begin() + currentBytesRead);
	_bytesRead += currentBytesRead;
}

void	CgiHandler::end()
{
	// std::cout << "Read everything from CGI" << std::endl;
	_cgiOutput.push_back('\0');
	_cgiOutput.shrink_to_fit();
}

// First we write all of our data to the CGI pipe
// After having everything written to it we can wait fot the output of the CGI
void	CgiHandler::handleWrite( void )
{
	size_t	bytesToWrite = WRITE_SIZE;

	if (bytesToWrite >= _cgiInput.size()) {
		_cgiInput.push_back('\0'); // add a terminator to the end so the CGI knows it can stop reading
		bytesToWrite = _cgiInput.size();
	}

	ssize_t bytesSent = write(_pipeWrite, _cgiInput.data(), bytesToWrite);
	if (bytesSent < 0)
		throw ( std::runtime_error("Failed to send response to client") );

	_cgiInput.erase(_cgiInput.begin(), _cgiInput.begin() + bytesSent);

	if (_cgiInput.size() == 0) {
		// wrote everything to pipe start reading the output
		_poll.addEvent(_pipeRead, POLLIN);
		_poll.removeEvent(_pipeWrite, POLLOUT);
		close(_pipeWrite);
		_pipeWrite = -1;
		return ;
	}
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
void	CgiHandler::startPythonCgi( HttpRequest& request, std::string script )
{
	this->clear();
	int pipeToCgi[2];
	int pipeFromCgi[2];
	
	// temp
	char	python[] = "python"; // execve cannot take a char const * char * so we have to do a lil workaround
	char	*args[] = { python, const_cast<char*>(script.c_str()), NULL };

	// Init pipes. Throws runtime_error on failure.
	if ( pipe( pipeToCgi ) == -1)
		throw ( std::runtime_error("Failed to create pipe") );
	if ( pipe( pipeFromCgi ) == -1 )
	{
		close( pipeToCgi[0] );
		close( pipeToCgi[1] );
		throw ( std::runtime_error("Failed to create pipe") );
	}

	// set pipes to non-blocking
	fcntl(pipeFromCgi[0], F_SETFL, O_NONBLOCK);
	fcntl(pipeFromCgi[1], F_SETFL, O_NONBLOCK);
	fcntl(pipeToCgi[0], F_SETFL, O_NONBLOCK);
	fcntl(pipeToCgi[1], F_SETFL, O_NONBLOCK);

	// Fork process. Throws runtime_error on failure.
	std::cout << GREEN "Starting CGI" RESET << std::endl;
	_forkPid = fork();
	if ( _forkPid == -1 )
	{
		close( pipeToCgi[0] );
		close( pipeToCgi[1] );
		close( pipeFromCgi[0] );
		close( pipeFromCgi[1] );
		throw ( std::runtime_error( "Failed to fork process" ) );
	}
	else if ( _forkPid == 0 ) // Child process
	{
		// Setup pipes in child process
		childInitPipes( pipeToCgi, pipeFromCgi );

		// Execute the CGI script
		execve( PYTHON_PATH, args, this->getEnvironmentVariables(request, script) );
		std::cerr << "Error executing python script" << std::endl;
		exit( 1 );
	}
	else // Parent process
	{
		// Setup pipes in parent process
		parentInitPipes( pipeToCgi, pipeFromCgi );
		_poll.addEvent(_pipeWrite, POLLOUT);
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
	_pipeWrite = pipeToCgi[1]; // Write end of pipeToCgi. Send data to CGI script.
	_pipeRead = pipeFromCgi[0]; // Read end of pipeFromCgi. Receive data from CGI script.
}