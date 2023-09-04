// #include <iostream>
// #include <cstdlib>
// #include <cstring>
// #include <unistd.h>

// int main(int argc, char **argv, char* envp[]) {
//     // Create pipes for communication
//     int inputPipe[2];  // Parent to Child (write to child)
//     int outputPipe[2]; // Child to Parent (read from child)

//     if (pipe(inputPipe) < 0 || pipe(outputPipe) < 0) {
//         perror("pipe");
//         return 1;
//     }

//     // Fork a child process
//     pid_t pid = fork();

//     if (pid < 0) {
//         perror("fork");
//         return 1;
//     }

//     if (pid == 0) { // Child Process
//         // Close unused pipe ends
//         close(inputPipe[1]);   // Close the write end of the input pipe
//         close(outputPipe[0]);  // Close the read end of the output pipe

//         // Redirect standard input and output to pipes
//         dup2(inputPipe[0], 0);   // Redirect stdin to the read end of inputPipe
//         dup2(outputPipe[1], 1);  // Redirect stdout to the write end of outputPipe

//     // Set environment variables if needed
//     // setenv("CONTENT_LENGTH", "123", 1);

//     // Construct the environment variable array
//     char* const env[] = {NULL}; // Add environment variables as needed

//     // Execute the CGI script
//     const char* program = "/WEBSERVfolder/webserv/mytest/upload_script.cgi"; // Provide the program name here
//     char* const args[] = {(char*)program, NULL}; // Program name followed by NULL
//     execve(program, args, env);
//     perror("execve"); // If execve fails
//         exit(1);
//     } else { // Parent Process
//         // Close unused pipe ends
//         close(inputPipe[0]);   // Close the read end of the input pipe
//         close(outputPipe[1]);  // Close the write end of the output pipe

//         // Send HTTP request data to the CGI script
//         const char* requestData = "Content-Type: multipart/form-data\r\n\r\nHello, CGI!";
//         write(inputPipe[1], requestData, strlen(requestData));

//         // Read and capture the CGI script's output
//         char buffer[1024];
//         std::string cgiOutput;
//         ssize_t bytesRead;
//         while ((bytesRead = read(outputPipe[0], buffer, sizeof(buffer))) > 0) {
//             cgiOutput.append(buffer, bytesRead);
//         }

//         // Construct an HTTP response based on CGI script output
//         std::string httpResponse = "HTTP/1.1 200 OK\r\n";
//         httpResponse += "Content-Length: " + std::to_string(cgiOutput.size()) + "\r\n";
//         httpResponse += "Content-Type: text/html\r\n\r\n";
//         httpResponse += cgiOutput;

//         // Send the HTTP response to the client
//         std::cout << httpResponse;
//     }

//     return 0;
// }


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

// #include "<Config.hpp>"
// #include <Server.hpp>
// #include <Location.hpp>
// #include <HttpServer.hpp>
// #include <Utils.hpp>
#include <iostream>
#include <unistd.h>


int main()
{
    // Config      config;
    // HttpServer  httpServer;

    int pipefd1[2];
    int pipefd2[2];
    char *args[] = { "python", "/Users/dkramer/WEBSERVfolder/webserv/mytest/upload_script.cgi", NULL };
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
