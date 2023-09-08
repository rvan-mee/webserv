/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   EventHandler.hpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: rvan-mee <rvan-mee@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/08/21 13:20:05 by rvan-mee      #+#    #+#                 */
/*   Updated: 2023/08/24 21:44:11 by rvan-mee      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef EVENTHANDLER_HPP
# define EVENTHANDLER_HPP

#include <CgiHandler.hpp>

typedef struct	s_requestData {
	std::vector<char>	buffer;
	bool				readHeaders;
	bool				contentLengthSet;
	int					headerSize;
	int					contentLength;
	int					totalBytesRead;
}	t_requestData;

class EventHandler
{
	private:
		EventHandler( void );

		int					_kqueueFd;
		int					_socketFd;
		CgiHandler			_cgi;
		t_requestData		_requestData;
		std::vector<char>	_socketWriteBuffer;
		int					_bytesWritten;

	public:
		EventHandler( int socketFd, int kqueueFd );
		~EventHandler();

		bool	isEvent( int fd );
		void	handleRead( int fd, Config &config );
		void	handleWrite( int fd );
};

#endif
