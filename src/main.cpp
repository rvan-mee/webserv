/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: cpost <cpost@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/07/06 14:16:31 by cpost         #+#    #+#                 */
/*   Updated: 2023/08/03 17:01:18 by cpost         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <Config.hpp>
#include <Server.hpp>
#include <Location.hpp>
#include <HttpServer.hpp>
#include <Utils.hpp>
#include <iostream>

int main()
{
    Config      config;
    HttpServer  httpServer;
    
    try 
    {
        /* Parse server config file. CONFIGFILE is defined in Config.hpp */
        config.parseConfig(CONFIGFILE);

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

