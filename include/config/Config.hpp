/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Config.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: cpost <cpost@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/06/23 11:25:45 by cpost         #+#    #+#                 */
/*   Updated: 2023/07/25 14:16:59 by rvan-mee      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

/**
 *  The config file is parsed into the following class structure
 * 
 *           +---------+
 *           | Config  |    -->   Class (Config.hpp/.cpp)
 *           +---------+
 *                |
 *     +----------+---------+
 *     |          |         |
 * +--------+ +--------+ +--------+
 * | Server | | Server | | Server |    --> Class (Server.hpp/.cpp)
 * +--------+ +--------+ +--------+
 *                |
 *      +---------+---------+
 *      |                   |
 * +----------+       +----------+ 
 * | Location |       | Location |    --> Class (location.hpp/.cpp)
 * +----------+       +----------+
 * 
 * The 'Config' object (config.hpp/.cpp) contains overall information that 
 * applies to the entire webserver configuration.
 * 
 * A 'Server' object (server.hpp/.cpp) containes the information that 
 * applies to (a) certain host adress(es).
 * 
 * A 'Location' object (location.hpp/cpp) contains information that applies
 * to a certain location/URL-path within a host adress.
 */

#ifndef CONFIG_HPP
# define CONFIG_HPP
# define CONFIGFILE "nginx.conf"

#include "Server.hpp"
#include "Location.hpp"
#include <vector>
#include <string>
#include <map>

/**
 * @brief The 'Config' class contains overall information that applies 
 * to the entire webserver configuration. This class will only be created
 * once.
 */
class Config
{
	private:

		std::vector<Server>				servers;
		std::vector<int>				listen;
		std::string						root;

	public:
		
		/*****************************
		 * Constructors / Destructor
		 ****************************/

		Config( void );
		~Config( void );

		/******************************
		 * Parser
		 *****************************/

		void	parseConfig( std::string const &filePath );
		void    tokenizeConfig( std::stringstream &fileContentsStream, 
					std::vector<std::string> &tokens );
		void	trimConfig( std::string &fileContentsString );
		void	parseInstruction( std::vector<std::string> &instruction );
		void	parseServerBlock( std::vector<std::string> &tokens );


		/******************************
		 * Setters
		 *****************************/

		void	parseListen( std::vector<std::string> &tokens );
		void	parseRoot( std::vector<std::string> &tokens );

		/******************************
		 * Getters
		 *****************************/

		Server				&getServer( std::string serverName );
		std::string			getRoot( void ) const;
		std::vector<int>	getListen( void ) const;

};

#endif
