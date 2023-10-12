/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: cpost <cpost@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/07/20 13:49:29 by cpost         #+#    #+#                 */
/*   Updated: 2023/09/21 16:05:59 by cpost         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <Server.hpp>
#include <Location.hpp>
#include <Utils.hpp>
#include <vector>
#include <string>
#include <map>
#include <stdexcept>
#include <regex>
#include <iostream>

/******************************
* Constructors / Destructor
*****************************/

/**
 * @brief Construct a new Server:: Server object
 */
Server::Server( void ) :
    serverName( std::vector<std::string>() ),
    listen( std::vector<int>() ),
    root( std::string() ),
    errorPage( std::map<int, std::string>() ),
    index( std::vector<std::string>() ),
    accessLog( std::string() ),
    errorLog( std::string() ),
    uploadsDir( std::string() ),
    redirect( std::string() )
{
}

/**
 * @brief Destroy the Server:: Server object
 */
Server::~Server()
{
}

/******************************
* Setters
*****************************/

/**
 * @brief Function to parse a location block from the config file.
 * @param tokens Vector of tokens. Contains the location block and everything behind.
 * @throw std::runtime_error if the location block is invalid.
 */
void    Server::parseLocationBlock( std::vector<std::string> &tokens )
{
    Location newLocation; // Create a new location block.

    /* Remove "Location" from the vector. */
    tokens.erase( tokens.begin() );

    /*  This will parse the URL's in the location block. */
    if ( tokens.size() == 1 || tokens[0] == "}" ) 
        throw ( std::runtime_error( "Invalid Location block in config file" ) );
    newLocation.parseLocationURL( tokens );

    /* Remove the opening bracket from the vector. */
    tokens.erase( tokens.begin() );

    // Loop through the tokens vector and parse the Location block.
    while ( tokens[0] != "}" )
    {
        if ( tokens.size() == 1 ) // If size is 1, then that means there's no closing bracket.
            throw ( std::runtime_error( "Invalid Location block in config file" ) );

        // Parse the instruction.
        if ( tokens[0] == "allow" ) 
            newLocation.parseAllow( tokens );
        else if ( tokens[0] == "fastcgi_pass" )
            newLocation.parseFastcgiPass( tokens );
        else if ( tokens[0] == "fastcgi_index" )
            newLocation.parseFastcgiIndex( tokens );
        else if ( tokens[0] == "fastcgi_param" )
            newLocation.parseFastcgiParam( tokens );
        else if ( tokens[0] == "include" )
            newLocation.parseInclude( tokens );
        else if ( tokens[0] == "autoindex")
            newLocation.parseAutoindex( tokens );
        else if ( tokens[0] == "alias" )
            newLocation.parseAlias( tokens );
        else if ( tokens[0] == "redirect" )
            newLocation.parseRedirect( tokens );
        else
            throw ( std::runtime_error( "Invalid instruction in Location block" ) );
    }
    // Remove the closing bracket from the tokens vector.
    tokens.erase( tokens.begin() );
    // Add the location block to the server block.
    this->locations.push_back( newLocation );
}

/**
 * @brief Parses the 'listen' instruction
 * @param tokens Vector with tokens
 */
void    Server::parseListen( std::vector<std::string> &tokens )
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
 * @brief Parses the 'server_name' instruction
 * @param tokens Vector with tokens
 */
void	Server::parseServerName( std::vector<std::string> &tokens )
{
    tokens.erase( tokens.begin() ); // Remove the 'server_name' token.
    while ( tokens[0] != ";" )
    {
        if ( tokens.size() <= 1 ) // If there is no ';' token, throw an error.
            throw ( std::runtime_error( "Invalid server_name instruction in config file" ) );

        // Add the server name to the server block.
        this->serverName.push_back( tokens[0] );

        // Remove the server name from the tokens vector.
        tokens.erase( tokens.begin() );
    }
    tokens.erase( tokens.begin() ); // Remove the ';' token.
}

/**
 * @brief Parses the 'root' instruction
 * @param tokens Vector with tokens
 */
void	Server::parseRoot( std::vector<std::string> &tokens )
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

void    Server::parseAutoindex ( std::vector<std::string> &tokens )
{
    tokens.erase( tokens.begin() ); // Remove the 'autoindex' token.

    for ( int i = 0; tokens[0] != ";"; i++ )
    {
        if ( tokens.size() <= 1 ) // If there is no ';' token, throw an error.
            throw ( std::runtime_error( "Invalid 'Autoindex' instruction in config file" ) );
        if ( i >= 1 )
            throw ( std::runtime_error( "Error: Multiple 'Autoindex' instructions in server block." ) );

        if ( tokens[0] == "on" )
            this->autoindex = true;
        else if ( tokens[0] == "off" )
            this->autoindex = false;
        else
            throw ( std::runtime_error( "Error: Invalid argument for 'Autoindex' instruction in location block." ) );
        tokens.erase( tokens.begin() );
    }
    tokens.erase( tokens.begin() ); // Remove the ';' token.
}

/**
 * @brief Parses the 'error_page' instruction
 * @param tokens Vector with tokens
 */
void	Server::parseErrorPage( std::vector<std::string> &tokens )
{
    std::vector<std::string>    tempTokens;
    std::string                 errorPageUrl;

    tokens.erase( tokens.begin() ); // Remove the 'error_page' token.

    /* Copy all tokens until the ';' token to a new temporary vector 'tempTokens'.
    Erase the copied elements from the original vector 'tokens'. */
    while ( tokens[0] != ";" )
    {
        if ( tokens.size() <= 1 ) // If there is no ';' token, throw an error.
            throw ( std::runtime_error( "Invalid Error_page instruction in config file" ) );
        tempTokens.push_back( tokens[0] );
        tokens.erase( tokens.begin() );
    }
    tokens.erase( tokens.begin() ); // Remove the ';' token.

    /* The last std::string of the tempTokens vector is the error page url. 
    So we set the value of this string to errorPageUrl for later use. 
    We then erase this element from the vector. After this, tempTokens
    only contains error codes */
    errorPageUrl = tempTokens.back();
    tempTokens.pop_back();

    /* Loop through all error codes in the tempTokens vector and add them to the
    errorPage map. */
    while ( tempTokens.size() > 0 )
    {
        try {
            this->errorPage.insert( std::pair<int, std::string>( std::stoi( tempTokens[0] ), errorPageUrl ) );
        }
        catch (const std::exception& e) {
            throw ( std::runtime_error( "Invalid error code in config file" ) );
        }
        tempTokens.erase( tempTokens.begin() );
    }
}

/**
 * @brief Parses the 'uploads_dir' instruction
 * @param tokens Vector with tokens
 */
void    Server::parseUploadsDir( std::vector<std::string> &tokens )
{
    tokens.erase( tokens.begin() ); // Remove the 'uploads_dir' token.

    for ( int i = 0; tokens[0] != ";"; i++ )
    {
        if ( tokens.size() <= 1 ) // If there is no ';' token, throw an error.
            throw ( std::runtime_error( "Invalid uploads_dir instruction in config file" ) );
        if ( i >= 1 )
            throw ( std::runtime_error( "Error: Multiple 'Uploads_dir' instructions in server block." ) );

        this->uploadsDir = tokens[0];
        tokens.erase( tokens.begin() );
    }
    tokens.erase( tokens.begin() ); // Remove the ';' token.
}

/**
 * @brief Parses the 'Index' instruction
 * @param tokens Vector with tokens
 */
void	Server::parseIndex( std::vector<std::string> &tokens )
{
    tokens.erase( tokens.begin() ); // Remove the 'index' token.

    // Loop through all tokens until the ';' token and add them to the index vector.
    while ( tokens[0] != ";" )
    {
        if ( tokens.size() <= 1 ) // If there is no ';' token, throw an error.
            throw ( std::runtime_error( "Invalid Index instruction in config file" ) );

        this->index.push_back( tokens[0] );
        tokens.erase( tokens.begin() );
    }
    tokens.erase( tokens.begin() ); // Remove the ';' token.
}

/**
 * @brief Parses the 'access_log' instruction
 * @param tokens Vector with tokens
 * @throws std::runtime_error if the server block contains multiple 'access_log' instructions.
 */
void	Server::parseAccessLog( std::vector<std::string> &tokens )
{
    tokens.erase( tokens.begin() ); // Remove the 'access_log' token.
    
    for ( int i = 0; tokens[0] != ";"; i++ )
    {
        if ( tokens.size() <= 1 ) // If there is no ';' token, throw an error.
            throw ( std::runtime_error( "Invalid access_log instruction in config file" ) );
        if ( i >= 1 ) // If there is more than one 'access_log' instruction, throw an error.
            throw ( std::runtime_error( "Error: Multiple 'access_log' instructions in server block." ) );
        this->accessLog = tokens[0];
        tokens.erase( tokens.begin() );
    }
    tokens.erase( tokens.begin() ); // Remove the ';' token.
}

/**
 * @brief Parses the 'error_log' instruction
 * @param tokens Vector with tokens
 * @throws std::runtime_error if the server block contains multiple 'error_log' instructions.
 */
void	Server::parseErrorLog( std::vector<std::string> &tokens )
{
    tokens.erase( tokens.begin() ); // Remove the 'error_log' token.

    for ( int i = 0; tokens[0] != ";"; i++ )
    {
        if ( tokens.size() <= 1 ) // If there is no ';' token, throw an error.
            throw ( std::runtime_error( "Invalid error_log instruction in config file" ) );
        if ( i >= 1 )
            throw ( std::runtime_error( "Error: Multiple 'error_log' instructions in server block." ) );
        this->errorLog = tokens[0];
        tokens.erase( tokens.begin() );
    }
    tokens.erase( tokens.begin() ); // Remove the ';' token.
}

/**
 * @brief Parse the redirect instruction in a location block.
 * 
 */
void    Server::parseRedirect( std::vector<std::string> &tokens )
{
    tokens.erase( tokens.begin() ); // Remove the 'redirect' token.

    for ( int i = 0; tokens[0] != ";"; i++ )
    {
        if ( tokens.size() <= 1 ) // If there is no ';' token, throw an error.
            throw ( std::runtime_error( "Invalid 'redirect' instruction in config file" ) );
        if ( i >= 1 )
            throw ( std::runtime_error( "Error: Multiple 'redirect' instructions in location block." ) );
        this->redirect = tokens[0];
        tokens.erase( tokens.begin() );
    }
    tokens.erase( tokens.begin() ); // Remove the ';' token.
}

/******************************
* Getters
*****************************/

/**
 * @brief Returns the location block that matches the given locationUrl.
 * @param locationUrl The locationUrl to match.
 * @return Location&
 * @throws std::runtime_error if no matching location block is found. 
 * IMPORTANT: Catch this error in the calling function.
 */
Location  &Server::getLocation( std::string locationUrl )
{
    std::vector<Location>::iterator     it;
    std::vector<std::string>::iterator  it2;
    std::vector<std::string>            urls;

    std::string temp = locationUrl;
    if ( temp == "/" )
    {
        for ( it = this->locations.begin(); it != this->locations.end(); it++ )
        {
            urls = it->getUrls();
            for ( it2 = urls.begin(); it2 != urls.end(); it2++ )
            {
                if ( *it2 == "" )
                    return ( *it );
            }
        }
    }
    for ( std::string temp = locationUrl; temp != ""; trimLocationUrl( temp ) )
    {
        for ( it = this->locations.begin(); it != this->locations.end(); it++ )
        {
            urls = it->getUrls();
            for ( it2 = urls.begin(); it2 != urls.end(); it2++ )
            {
                std::cout << "it2 " << *it2 << std::endl; 
                if ( *it2 == temp )
                    return ( *it );
            }
        }
        trimLocationUrl( temp );
    }
    throw ( std::runtime_error( "No matching location block found" ) );
}


/**
 * @brief Returns a vector with all server names of the current server block.
 * @return std::vector(std::string)
 */
std::vector<std::string>    Server::getServerNames( void ) const
{
    return ( this->serverName );
}

/**
 * @brief Get the root of the current server block
 * @return std::string 
 */
std::string Server::getRoot( void ) const
{
    return ( this->root );
}

/**
 * @brief Returns the uploads directory of the current server block.
 * @return std::string 
 */
std::string Server::getUploadsDir( void ) const
{
    return ( this->uploadsDir );
}

/**
 * @brief Returns a boolean indicating whether the given server block
 * has the autoindex enabled.
 * @return true if autoindex is enabled, false otherwise.
 */
bool    Server::getAutoindex( void ) const
{
    return ( this->autoindex );
}

/**
 * @brief Returns the error page for the given error code.
 * @param errorCode The error code for which the error page should be returned.
 * @return std::string. If no error page is found, an empty string is returned.
 */
std::string Server::getErrorPage( int errorCode ) const
{
    std::map<int, std::string>::const_iterator it;

    it = this->errorPage.find( errorCode );
    if ( it != this->errorPage.end() )
        return ( it->second );
    else
        return ( "" );
}

/**
 * @brief Returns the ports of the current server block.
 * @return std::vector(int)
 */
std::vector<int> Server::getListen( void ) const
{
    return ( this->listen );
}

/**
 * @brief Returns the path of the access log of a server block.
 * @return std::string 
 */
std::string Server::getAccessLog( void ) const
{
    return ( this->accessLog );
}

/**
 * @brief Returns the path of the eror log of a server block.
 * @return std::string 
 */
std::string Server::getErrorLog( void ) const
{
    return ( this->errorLog );
}

/**
 * @brief Returns the index paths of a server
 * @param server The server block to get the index paths from.
 * @return std::vector<std::string> 
 */
std::vector<std::string> Server::getIndex( void ) const
{
    return ( this->index );
}

/**
 * @brief Returns the redirect of the given location block.
 */
std::string Server::getRedirect( void ) const
{
    return ( this->redirect );
}
