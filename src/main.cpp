/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: cpost <cpost@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/07/06 14:16:31 by cpost         #+#    #+#                 */
/*   Updated: 2023/08/29 15:16:31 by cpost         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <Config.hpp>
#include <Server.hpp>
#include <Location.hpp>
#include <HttpServer.hpp>
#include <Utils.hpp>
#include <iostream>
#include <unistd.h>


int main()
{
    Config      config;
    HttpServer  httpServer;

    int pipefd1[2];
    int pipefd2[2];
    char *args[] = { "python", "/Users/cpost/Desktop/webserv/src/test.py", NULL };
    char *env[] = { NULL };

    try 
    {
        if (pipe(pipefd1) == -1)
            throw std::runtime_error("Failed to create pipe");
        if (pipe(pipefd2) == -1)
            throw std::runtime_error("Failed to create pipe");

        pid_t pid = fork();
        if (pid == -1)
        {
            std::cerr << "Error creating child process" << std::endl;
            return 1;
        }
        else if (pid == 0)
        {
            // Voer het Python-script uit
            execve(PYTHON_PATH, args, env);
            std::cerr << "Error executing Python script" << std::endl;
            return 1;
        }
        else
        {
            // Ouderproces: lees de leeszijde van de pipe en druk de inhoud af
            close(pipefd1[1]); // write
            close(pipefd2[0]); // read
            _pipeWrite = pipefd2[1];
            _pipeRead = pipefd1[0];
            char buffer[4024];
            ssize_t nread;
            while ((nread = read(pipefd[0], buffer, sizeof(buffer))) != 0)
            {
                std::cout.write(buffer, nread);
            }
            close(pipefd[0]);
        }
        // /* Parse server config file. CONFIGFILE is defined in Config.hpp */
        // config.parseConfig(CONFIGFILE);

        // /* Init http server */
        // httpServer.initServer( config );

    } 
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return (1);
    }
    return (0);
}

