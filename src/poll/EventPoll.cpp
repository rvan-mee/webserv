/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   EventPoll.cpp                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: rvan-mee <rvan-mee@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/09/19 19:46:55 by rvan-mee      #+#    #+#                 */
/*   Updated: 2023/09/21 21:15:37 by rvan-mee      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <EventPoll.hpp>
#include <unistd.h>
#include <iostream>

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


void	EventPoll::removeEvent(int fd, int eventType)
{
	t_fdToRemove	tmp;

	tmp.fd = fd;
	tmp.eventType = eventType;
	_removeList.push_back(tmp);
}

static void	eraseFromList(std::vector<pollfd>& list, t_fdToRemove fdToErase)
{
	size_t i = 0;

	while ( i < list.size()) {
		if (list[i].fd == fdToErase.fd && list[i].events == fdToErase.eventType)
			break ;
		i++;
	}

	if (i == list.size())
		return ;

	list.erase(list.begin() + i);
}

void	EventPoll::printList( void )
{
	char	*events[] = {
		[0] = "NO EVENT SET",
		[POLLIN] = "READ",
		[POLLOUT] = "WRITE",
		[POLLHUP] = "HANGUP"
	};

	for (size_t i = 0; i < _pollFds.size(); i++) {
		std::cout << "Event node: " << i << std::endl;
		std::cout << "fd:" << _pollFds[i].fd << std::endl;
		std::cout << "Events:" << events[_pollFds[i].events] << std::endl;
		std::cout << "Revents: " << _pollFds[i].revents << std::endl;
		std::cout << "Revents:" << events[_pollFds[i].revents] << std::endl;
		std::cout << std::endl;
	}
	
}

void	EventPoll::updateEventList( void )
{
	// remove every fd from the removal list
	while (_removeList.size() != 0) {
		t_fdToRemove	fdToRemove = _removeList.back();

		_removeList.pop_back();
		eraseFromList(_pollFds, fdToRemove);
	}

	// add every fd from the addition list
	while (_addList.size() != 0) {
		_pollFds.push_back(_addList.back());
		_addList.pop_back();
	}
}
