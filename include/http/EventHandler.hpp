/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   EventHandler.hpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: rvan-mee <rvan-mee@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/08/21 13:20:05 by rvan-mee      #+#    #+#                 */
/*   Updated: 2023/08/21 19:42:41 by rvan-mee      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef EVENTHANDLER_HPP
# define EVENTHANDLER_HPP

#include <CgiHandler.hpp>

class EventHandler
{
	private:
		EventHandler( void );

		int					_kqueueFd;
		int					_socketFd;
		CgiHandler			_cgi;
		std::vector<char>	_socketReadBuffer;
		std::vector<char>	_socketWriteBuffer;
		int					_bytesWritten;

	public:
		EventHandler( int socketFd, int kqueueFd );
		~EventHandler();

		bool	isEvent( int fd );
		void	handleRead( int fd );
		void	handleWrite( int fd );
};

#endif
