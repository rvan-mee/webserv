/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   EventPoll.hpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: rvan-mee <rvan-mee@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/09/19 19:46:11 by rvan-mee      #+#    #+#                 */
/*   Updated: 2023/09/20 21:21:07 by rvan-mee      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef EVENTPOLL_HPP
#  define EVENTPOLL_HPP

#include <poll.h>
#include <vector>

typedef struct	s_fdToAdd {
	int	fd;
	int	eventType;
}	t_fdToAdd;

class EventPoll
{
	public:
		EventPoll( void );
		~EventPoll();

		void					updateEventList( void );
		std::vector<pollfd>&	getEvents( void );
		void					addEvent(int fd, int eventType);
		void					removeEvent(int fd);

	private:
		std::vector<int>		_removeList;
		std::vector<pollfd>		_addList;
		std::vector<pollfd>		_pollFds;
};

#endif
