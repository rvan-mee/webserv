/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   KqueueUtils.hpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: rvan-mee <rvan-mee@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/08/24 16:12:54 by rvan-mee      #+#    #+#                 */
/*   Updated: 2023/08/25 17:43:24 by rvan-mee      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef KQUEUEUTILS_HPP
# define KQUEUEUTILS_HPP

void	addKqueueEventFilter( int kqueueFd, int eventFd, int filter );
void	deleteKqueueEventFilter( int kqueueFd, int eventFd, int filter );

#endif
