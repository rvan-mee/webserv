/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: cpost <cpost@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/07/06 14:16:31 by cpost         #+#    #+#                 */
/*   Updated: 2023/09/25 13:02:30 by dkramer       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <Config.hpp>
#include <Server.hpp>
#include <Location.hpp>
#include <HttpServer.hpp>
#include <Utils.hpp>
#include <iostream>
int main( int argc, char **argv)
{
    Config      config;
    HttpServer  httpServer;
    try
    {
        if (argc != 2)
            throw std::runtime_error( "Usage: ./webserv <configfile>" );
        /* Parse server config file. CONFIGFILE is defined in Config.hpp */
        config.parseConfig( argv[1] );
        /* Init http server */
        httpServer.initServer( config );
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return (1);
    }
    return (0);
}
