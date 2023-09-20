/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   EventPoll.cpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: rvan-mee <rvan-mee@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/09/19 19:46:55 by rvan-mee      #+#    #+#                 */
/*   Updated: 2023/09/20 21:21:21 by rvan-mee      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <EventPoll.hpp>
#include <unistd.h>

EventPoll::EventPoll()
{
}

EventPoll::~EventPoll()
{
}

std::vector<pollfd>&	EventPoll::getEvents( void )
{
	return (_pollFds);
}


void	EventPoll::addEvent(int fd, int eventType)
{
	pollfd	newEvent;

	newEvent.fd = fd;
	newEvent.events = eventType;
	newEvent.revents = 0;
	_addList.push_back(newEvent);
}


void	EventPoll::removeEvent(int fd)
{
	_removeList.push_back(fd);
}

static void	eraseFromList(std::vector<pollfd>& list, int fdToErase)
{
	size_t i = 0;

	while ( i < list.size()) {
		if (list[i].fd == fdToErase)
			break ;
		i++;
	}

	if (i == list.size())
		return ;

	list.erase(list.begin() + i);
}

void	EventPoll::updateEventList( void )
{
	// remove every fd from the removal list
	while (_removeList.size() != 0) {
		int	fdToRemove = _removeList.back();

		_removeList.pop_back();
		eraseFromList(_pollFds, fdToRemove);
	}

	// add every fd from the addition list
	while (_addList.size() != 0) {
		_pollFds.push_back(_addList.back());
		_addList.pop_back();
	}
}
