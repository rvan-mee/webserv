/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: cpost <cpost@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/07/20 13:49:03 by cpost         #+#    #+#                 */
/*   Updated: 2023/07/25 14:14:44 by rvan-mee      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

#include "Location.hpp"
#include <vector>
#include <string>
#include <map>

class Server
{
private:
    
    std::vector<Location>					locations;
    std::vector<std::string>				serverName;
	std::vector<int>						listen;
	std::string								root;
	std::map<int, std::string>				errorPage;
	std::vector<std::string>				index;
	std::string								accessLog;
	std::string								errorLog;

public:

    /*****************************
	* Constructors / Destructor
	****************************/

        Server( void );
        ~Server();

	/******************************
	* Setters
	*****************************/

        void	parseLocationBlock( std::vector<std::string> &tokens );
        void	parseListen( std::vector<std::string> &tokens );
        void	parseServerName( std::vector<std::string> &tokens );
        void	parseRoot( std::vector<std::string> &tokens );
        void	parseErrorPage( std::vector<std::string> &tokens );
        void	parseIndex( std::vector<std::string> &tokens );
        void	parseAccessLog( std::vector<std::string> &tokens );
        void	parseErrorLog( std::vector<std::string> &tokens );

	/******************************
	* Getters
	*****************************/

        Location                    &getLocation( std::string locationUrl );
        std::vector<std::string>	getServerNames( void ) const;
        std::string		            getRoot( void ) const;
        std::string		            getErrorPage( int errorCode ) const;
        std::vector<int>	        getListen( void ) const;
        std::string			        getAccessLog( void ) const;
		std::string			        getErrorLog( void ) const;
        std::vector<std::string>	getIndex( void ) const;
};


#endif