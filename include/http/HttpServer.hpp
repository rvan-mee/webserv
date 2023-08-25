/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HttpServer.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: cpost <cpost@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/07/27 10:17:28 by cpost         #+#    #+#                 */
/*   Updated: 2023/08/25 10:45:26 by dkramer       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPSERVER_HPP
# define HTTPSERVER_HPP

# define MAX_CONNECTIONS 100 // Used in HttpServer.cpp -> initServer()

# include <Config.hpp>
# include <EventHandler.hpp>
# include <netinet/in.h> // sockaddr_in
# include <sys/event.h>  // kqueue

class HttpServer
{
  private:
	int _serverSocket;
	sockaddr_in _address;
	int _kqueueFd;
	struct kevent _event[MAX_CONNECTIONS + 1];
	std::vector<EventHandler *> _eventList;

	int getEventIndex(int fd);

  public:
	/******************************
	* Constructors & Destructors
	*****************************/

	HttpServer(void);
	~HttpServer();

	/******************************
	* Server init
	*****************************/

	void initServer(Config &config);
	void createSocket(void);
	void bindSocket(Config &config);
	void setKqueue(void);
	void parseRequest(std::vector<char> buffer);
};

#endif
