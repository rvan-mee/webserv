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

class ClientHandler
{
	private:
		ClientHandler( void );

		void	readFromSocket( void );
		void	prepareNextRequest( void );

		int					_socketFd;
		CgiHandler			_cgi;
		t_requestData		_requestData;
		std::string			_response;
		Config&				_config;
		EventPoll&			_poll;
		bool				_doneReading;
		bool				_doneWriting;
		bool				_pollHupSet;
		HttpRequest			_request;

	public:
		ClientHandler( int socketFd, EventPoll& poll, Config& config );
		~ClientHandler();

		bool	doneWithRequest( void );
		bool	isEvent( int fd );
		bool	isSocketFd( int fd );
		void	handleRead( int fd );
		void	handleWrite( int fd );
		void	clear( void );
		void	setHup( void );
		void	endCgi( void );
};

#endif
