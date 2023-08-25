/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   CgiHandler.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: rvan-mee <rvan-mee@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/08/21 14:37:19 by rvan-mee      #+#    #+#                 */
/*   Updated: 2023/08/22 15:26:10 by rvan-mee      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

#include <vector>

class CgiHandler
{
	private:
		CgiHandler( void );

		void	parseCgiOutput( void );

		int					_kqueueFd;
		int					_pipeRead;
		int					_pipeWrite;
		std::vector<char>	_cgiOutput;
		int					_bytesRead;
		std::vector<char>	_cgiInput;
		int					_bytesWrote;
		int					_forkPid;

	public:
		CgiHandler( int kqueueFd );
		~CgiHandler();
	
		void	setWriteBuffer( std::vector<char>& buffer );

		bool	isEvent(int fd);
		void	handleRead( void );
		void	handleWrite( void );
};

#endif
