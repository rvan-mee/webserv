/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   CgiHandler.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: rvan-mee <rvan-mee@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/08/21 14:37:19 by rvan-mee      #+#    #+#                 */
/*   Updated: 2023/09/25 15:00:19 by rvan-mee      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

#include <EventPoll.hpp>
#include <vector>
#include <unistd.h>
#include <string>

class CgiHandler
{
	private:
		CgiHandler( void );

		void	parseCgiOutput( void );
		void	childInitPipes( int pipeToCgi[2], int pipeFromCgi[2] );
		void	parentInitPipes( int pipeToCgi[2], int pipeFromCgi[2] );

		EventPoll&			_poll;
		int					_pipeRead;
		int					_pipeWrite;
		std::vector<char>	_cgiOutput;
		int					_bytesRead;
		std::vector<char>	_cgiInput;
		int					_bytesWrote;
		bool				_doneReading;
		pid_t				_forkPid;

	public:
		CgiHandler( EventPoll& poll );
		~CgiHandler();
	
		void				setWriteBuffer( std::string& buffer );
		std::vector<char>&	getReadBuffer( void );

		bool	isEvent(int fd);
		bool	isRunning( void );
		bool	isDoneReading( void );
		void	handleRead( void );
		void	handleWrite( void );
		void	clear( void );
		void	end( void );

		void	startPythonCgi( std::string script );
};

#endif
