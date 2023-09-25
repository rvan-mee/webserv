/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HttpResponse.hpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: cpost <cpost@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/07/27 10:17:28 by cpost         #+#    #+#                 */
/*   Updated: 2023/09/21 12:04:45 by dkramer       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

# include <fstream>
# include <iostream>
# include <sstream>
# include <string>
# include <vector>
# include <Config.hpp>
/**
 * @brief HttpResponse class to store the data in the class variables
 * 
 */
class HttpResponse
{
  public:
	HttpResponse();
	void setContentType(std::string contentType);
	void addLineToBody(std::string line);
	void printAll();
	void setError(int statusCode, std::string _reason_phrase);
	std::string buildResponse( Server server);
	void	setMessageBody( Server server);

  private:
	int _status_code;
	std::string _reason_phrase;
	std::string _message_body;
	std::string _content_type;
};

#endif
