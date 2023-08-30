/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   KqueueUtils.hpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: rvan-mee <rvan-mee@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/08/24 16:12:54 by rvan-mee      #+#    #+#                 */
/*   Updated: 2023/08/28 18:33:37 by rvan-mee      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef KQUEUEUTILS_HPP
# define KQUEUEUTILS_HPP

#include <sys/event.h>

void	addKqueueEventFilter( int kqueueFd, int eventFd, int filter );
void	deleteKqueueEventFilter( int kqueueFd, int eventFd, int filter );

#endif
