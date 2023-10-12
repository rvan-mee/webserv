/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Config.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: cpost <cpost@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/06/26 13:10:22 by cpost         #+#    #+#                 */
/*   Updated: 2023/09/25 14:52:55 by rvan-mee      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <Config.hpp>
#include <Server.hpp>
#include <Location.hpp>
#include <Utils.hpp>
#include <regex>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>

/******************************
* Constructors & Destructors
*****************************/

Config::Config() :
    servers( std::vector<Server>() ),
    listen( std::vector<int>() ),
    root( std::string() )
{
}

Config::~Config()
{
}

/******************************
* Parser functions
*****************************/

/**
 * @brief Starting function to parse the config file to config object. 
 * This function calls the necessary functions to parse the config file.
 * @param path Path to the config file
 * @throw std::runtime_error if the config file is invalid or the 
 * file at filePath could is empty or could not be read.
 */
void  Config::parseConfig( std::string const &filePath )
{
    std::stringstream fileContentsStream( readFile( filePath ) ); // Stringstream to store the file contents in.

    // Check if the file is empty or could not be read. Throw an error if so.
    if ( fileContentsStream.str().empty() ) 
        throw ( std::runtime_error( "Config file empty or could not be read" ) );

    /* Now we're going to remove all unnecessary things from the config file
    and convert it to a more workable vector of tokens. */
    std::vector<std::string>    tokens; // The tokenized config file will be stored in this vector.
    this->tokenizeConfig( fileContentsStream, tokens ); // tokenize the config file. 
    if ( tokens.empty() ) // Check if the vector is empty. Throw an error if so.
        throw ( std::runtime_error( "Tokenizing config file failed" ) );

    /* In this loop we're going to parse the config file. We're going to do this by 
    creating a vector of strings that contains a single instruction from the config file.
    This vector is then passed to the parseInstruction function. */
    std::vector<std::string>    instruction; // Will contain a single instuction from the config file.
    while ( tokens.size() > 0 ) // Loop until the vector is empty.
    {
        // Parse Server block
        if ( tokens[0] == "server" )
        {
            /* There are three things that must be true for a valid server block to exist: the
            token vector must be at least 3 elements long, the first token must be "server" and
            second token must be a "{". The instruction vector must be empty, because there
            can't be any unparsed instructions before the server block. */
            if ( tokens.size() <= 2 || tokens[1] != "{" || !instruction.empty() )
                throw ( std::runtime_error( "Invalid server block in config file" ) );

            /* After the parseServerBlock function, the tokens vector will start with the 
            first token after the closing bracket of the parsed server block. */
            this->parseServerBlock( tokens ); 
        }

        // // Parse Location block (not implemented yet, since not sure if it's needed to parse a location block
        // // at config file level (instead of server level).)
        // else if ( tokens[0] == "location")
        // {
        //     /* The instruction vector must be empty, because there
        //     can't be any unparsed instructions before the location block. */
        //     if ( !instruction.empty() || tokens.size() <= 2 )
        //         throw ( std::runtime_error( "Invalid location block in config file" ) );

        //     /* After the parseLocationBlock function, the tokens vector will start with the
        //     first token after the closing bracket of the parsed location block. */
        //     this->parseLocationBlock( tokens );
        // }

        // Parse normal instruction
        else
        {
            instruction.push_back( tokens[0] ); // Add token to the instruction vector
            if ( tokens[0] == ";" ) 
            {
                this->parseInstruction( instruction ); // Parse the instruction.
                instruction.clear(); // Clear the instruction vector. This is so we can use it again for the next instruction.
            }
            else if ( tokens.size() == 1 ) // If the instruction doesn't end with a semicolon, but the vector is empty, then throw an error.
                throw ( std::runtime_error( "Invalid instruction in config file" ) );
            tokens.erase( tokens.begin() ); // Remove the first element from the token vector.
        }
    }
}

/**
 * @brief Function to parse a single instruction from the config file.
 * @param tokens (Instruction) Vector of strings that contains a single instruction from the config file.
 * @throw std::runtime_error if the instruction is invalid.
 */
void    Config::parseInstruction( std::vector<std::string> &tokens )
{
    if ( tokens[0] == "listen" )
        this->parseListen( tokens );
    else if ( tokens[0] == "root" )
        this->parseRoot( tokens );
    else if ( tokens[0] == "client_max_body_size" )
        this->parseClientMaxBodySize( tokens );
    else
        throw ( std::runtime_error( "Invalid instruction in config file" ) );
}

/**
* @brief Function to convert the config file to a vector of tokens.
* @param fileContentsStream The stringstream that contains the config file.
*/
void    Config::tokenizeConfig( std::stringstream &fileContentsStream, 
            std::vector<std::string> &tokens )
{
    /* First we're going to remove all unnecessary things from the config file. 
    This means: 1. Comments, 2. Empty lines, 3. Unnecessary spaces. Besides that
    we're going to add spaces around semicolons and brackets. This makes tokenizing
    easier later on. */
    std::string fileContentsString = fileContentsStream.str(); // Convert the stringstream to a string.
    this->trimConfig( fileContentsString ); // Trim the string.

    /* Move the trimmed string back to the stringstream. Since we want to keep the address of 
    the stringstream the same, we're using std::stringstream::swap. */
    std::stringstream( fileContentsString ).swap( fileContentsStream );

    // Tokenize the config file and store the tokens in the tokens vector.
    std::string temp; // Temporary string to store the tokens in.
    while ( std::getline( fileContentsStream, temp, ' ' ) ) // Tokenize the config file. Split on spaces.
        tokens.push_back( temp ); // Add the token to the tokens vector.
}

/**
 * @brief Function to remove unnecessary things from the config file string.
 * @param fileContentString The string to trim.
 */
void    Config::trimConfig( std::string &fileContentString )
{
    // Replace all whitespace for spaces.
    fileContentString = std::regex_replace( fileContentString, std::regex("[\\t\\r]+"), " " );

    // Remove all unnecessary trailing and leading spaces from the line.
    fileContentString = std::regex_replace( fileContentString, std::regex(" +\\n|\\n +"), "\n" );

    // Delete all comments, also inline.
    fileContentString = std::regex_replace( fileContentString, std::regex("#.*?(?=\\n)"), "" );

    // Delete all newlines.
    fileContentString = std::regex_replace( fileContentString, std::regex("\\n"), "" );

    // Add a space before and after '{' and '}' This is needed for tokenizing.
    fileContentString = std::regex_replace( fileContentString, std::regex("\\{"), " { " );
    fileContentString = std::regex_replace( fileContentString, std::regex("\\}"), " } " );

    // Add a space before and after semicolon if there is none. This is needed for tokenizing.
    fileContentString = std::regex_replace( fileContentString, std::regex("([^\\s;])\\s*;"), "$1 ; " );

    // Replace all multiple spaces with one space.
    fileContentString = std::regex_replace( fileContentString, std::regex("[ ]+"), " " );
}


/**
 * @brief Function to parse a server block from the config file. 
 * IMPORTANT:
 * After parsing the server block, the function will have deleted everything 
 * in the tokens vector up to the closing bracket of the server block.
 * @param tokens Vector of tokens. Contains the server block and everything behind
 * that server block.
 * @throws std::runtime_error if the server block is invalid.
 */
void    Config::parseServerBlock( std::vector<std::string> &tokens )
{
    Server  newServer; // Create a new server class.

    /* Remove "Server" and "{" from the vector. Second parameter of erase is 
    the new beginning of the vector, which will be the first word after "{" */
    tokens.erase( tokens.begin(), tokens.begin() + 2 );

    // Loop through the tokens vector and parse the server block.
    while ( tokens[0] != "}" )
    {
        if ( tokens.size() == 1 ) // If size is 1, then that means there's no closing bracket.
            throw ( std::runtime_error( "Invalid server block in config file" ) );

        // Parse the instruction.
        if ( tokens[0] == "location")
            newServer.parseLocationBlock( tokens );
        else if ( tokens[0] == "listen" )
            newServer.parseListen( tokens );
        else if ( tokens[0] == "server_name" )
            newServer.parseServerName( tokens );
        else if ( tokens[0] == "error_page" )
            newServer.parseErrorPage( tokens );
        else if ( tokens[0] == "root" )
            newServer.parseRoot( tokens );
        else if ( tokens[0] == "index" )
            newServer.parseIndex( tokens );
        else if ( tokens[0] == "access_log")
            newServer.parseAccessLog( tokens );
        else if ( tokens[0] == "error_log")
            newServer.parseErrorLog( tokens );
        else if ( tokens[0] == "autoindex" )
            newServer.parseAutoindex( tokens );
        else if ( tokens[0] == "uploads_dir" )
            newServer.parseUploadsDir( tokens );
        else if ( tokens[0] == "redirect" )
            newServer.parseRedirect( tokens );
        else
            throw ( std::runtime_error( "Invalid instruction in server block" ) );
    }
    // Remove the closing bracket from the tokens vector.
    tokens.erase( tokens.begin() );
    // Add the server block to the config object.
    this->servers.push_back( newServer );
}

/******************************
* Setters
*****************************/

/**
 * @brief Parses the 'listen' instruction
 * @param tokens Vector with tokens
 */
void    Config::parseListen( std::vector<std::string> &tokens )
{
    tokens.erase( tokens.begin() ); // Remove the 'listen' token.
    while ( tokens[0] != ";" )
    {
        if ( tokens.size() <= 1 ) // If there is no ';' token, throw an error.
            throw ( std::runtime_error( "Invalid listen instruction in config file" ) );

        // Try to convert the token to an integer. If it fails, throw an error.
        try {
            this->listen.push_back( std::stoi( tokens[0] ) ); }
        catch(const std::exception& e) {
            throw ( std::runtime_error( "Invalid port number in config file" ) );
        }
        tokens.erase( tokens.begin() );
    }
    tokens.erase( tokens.begin() ); // Remove the ';' token.
}

/**
 * @brief Parses the 'root' instruction
 * @param tokens Vector with tokens
 */
void	Config::parseRoot( std::vector<std::string> &tokens )
{
    tokens.erase( tokens.begin() ); // Remove the 'root' token.
    while ( tokens[0] != ";" )
    {
        if ( tokens.size() <= 1 ) // If there is no ';' token, throw an error.
            throw ( std::runtime_error( "Invalid root instruction in config file" ) );

        this->root = tokens[0];
        tokens.erase( tokens.begin() );
    }
    tokens.erase( tokens.begin() ); // Remove the ';' token.
}

void    Config::parseClientMaxBodySize( std::vector<std::string> &tokens )
{
    tokens.erase( tokens.begin() ); // Remove the 'client_max_body_size' token.

    long number = 0;
    while ( tokens[0] != ";")
    {
        size_t i = 0;
        /* First, we're going to parse the numbers. We're going to do this by looping
        through the string and checking if the current character is a digit. If it is,
        we're going to convert it to an integer and add it to the number variable. */
        try {
            number = std::stoi(tokens[0], &i, 10);
        }
        catch ( std::exception &e ) {
            throw ( std::runtime_error( "Invalid client_max_body_size instruction in config file" ) );
        }
        if ( number < 0 )
            throw ( std::runtime_error( "Invalid client_max_body_size instruction in config file" ) );
        /* Now we're going to check whether the current character is a semicolon,
        or a 'K' or 'M'. If it's a semicolon, we're done. If it's a 'K' or 'M',
        we're going to multiply the number by 1024 or 1024 * 1024 respectively. */
        if ( tokens[0][i] == ';')
            break;
        else if ( tokens[0][i] == 'K' )
            number *= 1024;
        else if ( tokens[0][i] == 'M' )
            number *= 1024 * 1024;
        else
            throw ( std::runtime_error( "Invalid client_max_body_size instruction in config file" ) );
        i++;
        if ( tokens[0][i] != ';' )
            throw ( std::runtime_error( "Invalid client_max_body_size instruction in config file" ) );
        
        this->clientMaxBodySize = number; // Set the clientMaxBodySize variable.

        tokens.erase( tokens.begin() ); // Remove the number from the tokens vector.
    }
    tokens.erase( tokens.begin() ); // Remove the ';' token.
}

/******************************
* Getters
*****************************/

/**
 * @brief Get the server block with the given server name
 * @param serverName The name of the server block
 * @return The server block corresponding to the given server name. If the
 * server name is not found, the default server is returned.
 * @throw std::runtime_error if 'serverName' is not found, 
 * and also if no default server is found.
 */
Server &Config::getServer( std::string serverName )
{
    std::vector<Server>::iterator       defaultServer;
    std::vector<Server>::iterator       it;
    std::vector<std::string>::iterator  it2;
    std::vector<std::string>            serverNames;

    defaultServer = this->servers.end();
    for ( it = this->servers.begin(); it != this->servers.end(); it++ )
    {
        serverNames = it->getServerNames();
        for ( it2 = serverNames.begin(); it2 != serverNames.end(); it2++ )
        {
            if ( *it2 == serverName ) {
                return ( *it );
            }
            if ( *it2 == "_" ) {
                defaultServer = it;
            }
        }
    }
    if ( defaultServer != this->servers.end() )
        return ( *defaultServer );
    else
        throw ( std::runtime_error( "Error: No default server found." ) );
}

/**
 * @brief Get the root of the config.
 * @return std::string 
 */
std::string& Config::getRoot( void )
{
    return ( this->root );
}

/**
 * @brief Returns the ports of the config.
 * @return std::vector<int> 
 */
std::vector<int>& Config::getListen( void )
{
    return ( this->listen );
}

/**
 * @brief Returns the client_max_body_size of the config.
 * @return unsigned long 
 */
unsigned long Config::getClientMaxBodySize( void ) const
{
    return ( this->clientMaxBodySize );
}

/**
 * @brief Returns the servers of the config.
 * @return std::vector<Server> 
 */
std::vector<Server>&    Config::getAllServers( void )
{
    return (this->servers);
}
