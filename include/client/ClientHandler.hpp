/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ClientHandler.hpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: rvan-mee <rvan-mee@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/08/21 13:20:05 by rvan-mee      #+#    #+#                 */
/*   Updated: 2023/08/25 16:31:18 by rvan-mee      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENTHANDLER_HPP
# define CLIENTHANDLER_HPP

#include <EventPoll.hpp>
#include <CgiHandler.hpp>
#include <HttpRequest.hpp>
#include <Config.hpp>
#include <string>
#include <chrono>
#include <ctime> 

#define REQUEST_TIMEOUT 10000000 // 10 seconds

typedef struct	s_requestData {
	std::vector<char>	buffer;
	std::vector<char>	prevBuffer;
	std::vector<char>	chunkedBuffer;
	int					chunkSize;
	bool				movedHeaders;
	bool				readHeaders;
	bool				contentLengthSet;
	bool				chunkedEncoded;
	int					headerSize;
	int					contentLength;
	int					totalBytesRead;
}	t_requestData;


typedef std::chrono::steady_clock::time_point serverTime;

class ClientHandler
{
	private:
		ClientHandler( void );

		void	readFromSocket( void );
		void	prepareNextRequest( void );
		void	resetTimeOut( void );
		void	setTimeOutResponse( bool cgiRunning );

		int					_socketFd;
		int					_port;
		CgiHandler			_cgi;
		t_requestData		_requestData;
		std::string			_response;
		Config&				_config;
		EventPoll&			_poll;
		bool				_doneReading;
		bool				_doneWriting;
		bool				_pollHupSet;
		bool				_timeOutSet;
		serverTime			_timeOutStart;
		HttpRequest			_request;

	public:
		ClientHandler( int socketFd, EventPoll& poll, Config& config, int port );
		~ClientHandler();

		int		getSocketFd( void ) { return (_socketFd); };
		bool	isDoneWriting( void ) { return (_doneWriting); };
		bool	isTimedOut( void ) { return (_timeOutSet); } ;
		bool	doneWithRequest( void ) { return (_doneReading && _cgi.isRunning() == false); };
		bool	isCgiRunning( void ) { return (_cgi.isRunning()); } ;
		bool	isSocketFd( int fd ) { return (fd == _socketFd); };
		bool	isEvent( int fd ) { return (fd == _socketFd || _cgi.isEvent(fd)); };
		bool	getHangup( void ) { return (_pollHupSet); }

		bool	checkTimeOut( serverTime& currentTime );
		void	handleRead( int fd );
		void	handleWrite( int fd );
		void	clear( void );
		void	setHup( void );
		void	endCgi( void );
};

#endif
