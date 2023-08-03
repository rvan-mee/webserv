/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: cpost <cpost@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/07/06 14:16:31 by cpost         #+#    #+#                 */
/*   Updated: 2023/08/03 16:17:10 by cpost         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <Config.hpp>
#include <Server.hpp>
#include <Location.hpp>
#include <HttpServer.hpp>
#include <Utils.hpp>
#include <iostream>

// Global variable for signal handling. 
volatile    sig_atomic_t gSignalStatus = 0;

// Signal handler for SIGINT
void signalHandler(int signal)
{
    gSignalStatus = signal;
}

int main()
{
    Config      config;
    HttpServer  httpServer;
    
    try 
    {
        /* Set signal handler */
        signal(SIGINT, signalHandler);

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

    // std::cout << config.getServer("example.com").getLocation("/files/photos/main/hello/cute_cat.jpg").getFastcgiPass() << std::endl;

    // std::cout << config.getServer("example.com").getListen()[0] << std::endl;

    // std::cout << config.getServer("example.com").getAccessLog() << std::endl;
    // std::cout << "ROOT: " << config.getRoot() << std::endl;
    // std::cout << "ADDRES: " << &config.getServer("example.com") << std::endl;
    // std::cout << "Error log: " << config.getServer("example.com").getErrorLog() << std::endl;
    // std::cout << "Error Page: " << config.getServer("test").getErrorPage( 404 ) << std::endl;
    // std::cout << "Error Page: " << config.getServer("test").getErrorPage( 502 ) << std::endl;
    // std::cout << "ADDRES: " << &config.getServer("example.com") << std::endl;
    // std::cout << "ADDRES: " << &config.getServer("_") << std::endl;
    return (0);
}

// g++ Config.hpp Config.cpp utils.cpp utils.hpp parseTestMain.cpp 