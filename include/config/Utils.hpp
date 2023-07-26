/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   utils.hpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: cpost <cpost@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/07/03 18:10:59 by cpost         #+#    #+#                 */
/*   Updated: 2023/07/24 15:26:24 by cpost         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
# define UTILS_HPP

#include "Config.hpp"
#include <string>
#include <fstream>
#include <sstream>

std::string readFile( const std::string& filePath );
void        trimLine( std::string &line );
bool        trimLocationUrl( std::string& input );

#endif
