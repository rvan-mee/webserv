/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   KqueueUtils.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: rvan-mee <rvan-mee@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/08/24 16:13:51 by rvan-mee      #+#    #+#                 */
/*   Updated: 2023/08/25 21:05:50 by rvan-mee      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <KqueueUtils.hpp>
#include <sys/event.h>
#include <stdexcept>
#include <iostream>


void	addKqueueEventFilter( int kqueueFd, int eventFd, int filter )
{
	std::cout << "Adding event: " << (filter == EVFILT_READ ? "READ" : "WRITE") << std::endl;
	struct kevent	evSet;

	// Adding the event filter to kqueue, using EV_CLEAR it removes it after it has been handled.
	EV_SET( &evSet, eventFd, filter, EV_ADD | EV_CLEAR, 0, 0, NULL );
	if ( kevent( kqueueFd, &evSet, 1, NULL, 0, NULL ) < 0 )
		throw ( std::runtime_error( "Failed to add an event filter to kqueue" ) );
}

void	deleteKqueueEventFilter( int kqueueFd, int eventFd, int filter )
{
	std::cout << "Deleting event: " << (filter == EVFILT_READ ? "READ" : "WRITE") << std::endl;
	struct kevent	evSet;

	EV_SET( &evSet, eventFd, filter, EV_DELETE, 0, 0, NULL );
	if ( kevent( kqueueFd, &evSet, 1, NULL, 0, NULL ) < 0 )
		throw ( std::runtime_error( "Failed to add an event filter to kqueue" ) );
}

