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
	_addQueue.push_back(newEvent);
}


void	EventPoll::removeEvent(int fd, int eventType)
{
	t_fdToRemove	tmp;

	tmp.fd = fd;
	tmp.eventType = eventType;
	_removeQueue.push_back(tmp);
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
	std::string	events;
	std::string	revents;

	for (size_t i = 0; i < _pollFds.size(); i++) {
		int	eventNum = _pollFds[i].events;
		int	reventNum = _pollFds[i].revents;

		if (eventNum == 0)
			events = "No event";
		if (eventNum & POLLIN)
			events += "POLLIN ";
		if (eventNum & POLLRDHUP)
			events += "POLLRDHUP";
		if (eventNum & POLLOUT)
			events += "POLLOUT";

		if (reventNum == 0)
			revents = "No revent";
		if (reventNum & POLLHUP)
			revents += "POLLHUP ";
		if (reventNum & POLLIN)
			revents += "POLLIN ";
		if (reventNum & POLLRDHUP)
			revents += "POLLRDHUP";
		if (reventNum & POLLOUT)
			revents += "POLLOUT";

		std::cout << "Event index: " << i << std::endl;
		std::cout << "fd: " << _pollFds[i].fd << std::endl;
		std::cout << "Events: " << events << std::endl;
		std::cout << "Revents: " << revents << std::endl;
		events.clear();
		revents.clear();
		std::cout << std::endl;
	}
	
}

void	EventPoll::updateEventList( void )
{
	// remove every fd from the removal list
	while (_removeQueue.size() != 0) {
		t_fdToRemove	fdToRemove = _removeQueue.back();

		_removeQueue.pop_back();
		eraseFromList(_pollFds, fdToRemove);
	}

	// add every fd from the addition list
	while (_addQueue.size() != 0) {
		_pollFds.push_back(_addQueue.back());
		_addQueue.pop_back();
	}
}
