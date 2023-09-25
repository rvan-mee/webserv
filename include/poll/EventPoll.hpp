/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   EventPoll.hpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: rvan-mee <rvan-mee@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/09/19 19:46:11 by rvan-mee      #+#    #+#                 */
/*   Updated: 2023/09/21 21:11:49 by rvan-mee      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef EVENTPOLL_HPP
#  define EVENTPOLL_HPP

#include <poll.h>
#include <vector>

typedef struct	s_fdToRemove {
	int	fd;
	int	eventType;
}	t_fdToRemove;

class EventPoll
{
	public:
		EventPoll( void );
		~EventPoll();

		void					updateEventList( void );
		std::vector<pollfd>&	getEvents( void );
		void					addEvent(int fd, int eventType);
		void					removeEvent(int fd, int eventType);
		void					printList( void );

	private:
		std::vector<t_fdToRemove>	_removeList;
		std::vector<pollfd>			_addList;
		std::vector<pollfd>			_pollFds;
};

#endif
