/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HttpServer.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: cpost <cpost@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/07/27 10:17:28 by cpost         #+#    #+#                 */
/*   Updated: 2023/09/25 15:04:21 by rvan-mee      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPSERVER_HPP
# define HTTPSERVER_HPP

# define MAX_CONNECTIONS 100 // Used in HttpServer.cpp -> initServer()

# include <Config.hpp>
# include <ClientHandler.hpp>
# include <EventPoll.hpp>
# include <netinet/in.h> // sockaddr_in

class HttpServer
{
  private:
	std::vector<int>				_serverSockets;
	std::vector<int>				_ports;
	std::vector<ClientHandler *>	_eventList;
	EventPoll						_poll;

	int		getEventIndex(int fd);
	void	removeClient(int eventIndex, int eventFd);

  public:
	/******************************
	* Constructors & Destructors
	*****************************/

	HttpServer(void);
	~HttpServer();

	/******************************
	* Server init
	*****************************/

	bool	isServerSocket( int fd );
	void	initServer(Config &config);
	void	createSockets(void);
	void	bindSockets(void);
	void	listenToSockets(void);
	void	pollSockets(void);
	void	parseRequest(std::vector<char> buffer);
	void	closeServerSockets( void );
	void	setPorts( Config &config );
};

#endif
