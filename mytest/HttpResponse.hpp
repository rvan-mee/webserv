/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HttpResponse.hpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: cpost <cpost@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/07/27 10:17:28 by cpost         #+#    #+#                 */
/*   Updated: 2023/08/11 13:04:53 by dkramer       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
/**
 * @brief HttpServer class to parse the request and store the data in the class variables
 * 
 */
class HttpResponse
{
	public:
		void	setContentType(std::string contentType);
		void    addLineToBody(std::string line);
		void    printAll();
		// void	setStatusCode(int statusCode);
		// void	setReasonPhrase(std::string);
		void	setError(int statusCode, std::string _reason_phrase);
		std::string	buildResponse();
	private:
		int _status_code;
        std::string _reason_phrase;
		std::string	_message_body;
		std::string	_content_type;
};

#endif
