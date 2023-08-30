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

#include <CgiHandler.hpp>
#include <string>

typedef struct	s_requestData {
	std::vector<char>	buffer;
	bool				readHeaders;
	bool				contentLengthSet;
	int					headerSize;
	int					contentLength;
	int					totalBytesRead;
}	t_requestData;

class ClientHandler
{
	private:
		ClientHandler( void );

		int					_kqueueFd;
		int					_socketFd;
		CgiHandler			_cgi;
		t_requestData		_requestData;
		std::string			_response;
		int					_bytesWritten;

	public:
		ClientHandler( int socketFd, int kqueueFd );
		~ClientHandler();

		bool	isEvent( int fd );
		void	handleRead( int fd );
		void	handleWrite( int fd );
};

#endif
