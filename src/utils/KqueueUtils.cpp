/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   KqueueUtils.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: rvan-mee <rvan-mee@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/08/24 16:13:51 by rvan-mee      #+#    #+#                 */
/*   Updated: 2023/08/24 16:24:24 by rvan-mee      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <KqueueUtils.hpp>
#include <sys/event.h>
#include <stdexcept>

void	addKqueueEventFilter( int kqueueFd, int eventFd, int filter )
{
	struct kevent	evSet;
	// Adding the event filter to kqueue, using EV_CLEAR it removes it after it has been handled.
	EV_SET( &evSet, eventFd, filter, EV_ADD | EV_CLEAR, 0, 0, NULL );
	if ( kevent( kqueueFd, &evSet, 1, NULL, 0, NULL ) < 0 )
		throw ( std::runtime_error( "Failed to add an event filter to kqueue" ) );
}

