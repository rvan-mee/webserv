/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   FileHandler.hpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: rvan-mee <rvan-mee@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/08/27 14:05:49 by rvan-mee      #+#    #+#                 */
/*   Updated: 2023/08/30 12:41:46 by rvan-mee      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef FILEHANDLER_HPP
# define FILEHANDLER_HPP

#include <string>
#include <vector>

typedef enum	e_fileState {
	BUSY,
	DONE,
}				t_fileState;

class FileHandler
{
	private:
		FileHandler( void );
		int					_fd;
		int					_kqueueFd;
		int					_fileSize;
		ssize_t				_bytesRead;
		std::string			_fileName;
		std::vector<char>	_readBuffer;
		std::string			_writeBuffer;

	public:
		FileHandler( int kqueueFd );
		~FileHandler();

		void				createFile( std::string& fileName, std::string& writeBuffer ); // throws exception if file already exists
		void				openFile( std::string& fileName, int mode ); // throws an exception if the file does not have the right permissions or if opening failed
		bool				isEvent( int fd );

		std::vector<char>&	getReadBuffer( void );

		t_fileState			handleRead( void );
		t_fileState			handleWrite( void );

};

#endif
