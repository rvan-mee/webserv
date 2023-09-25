/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   CgiHandler.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: rvan-mee <rvan-mee@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/08/21 14:37:19 by rvan-mee      #+#    #+#                 */
/*   Updated: 2023/08/29 15:15:10 by cpost         ########   odam.nl         */
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
		void	childInitPipes( int pipeToCgi[2], int pipeFromCgi[2] );
		void	parentInitPipes( int pipeToCgi[2], int pipeFromCgi[2] );

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

		void	startPythonCgi( void );
};

#endif
