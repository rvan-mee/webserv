/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Location.cpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: cpost <cpost@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/07/24 12:42:47 by cpost         #+#    #+#                 */
/*   Updated: 2023/09/21 15:58:16 by cpost         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Location.hpp"
#include "Server.hpp"
#include <string>
#include <vector>
#include <regex>
#include <iostream>

/******************************
* Constructors / Destructor
*****************************/

Location::Location( void ) :
    url( std::vector<std::string>() ),
    allowGet( false ),
    allowPost( false ),
    allowDelete( false ),
    autoindex( true ),
    fastcgiPass( std::string() ),
    fastcgiIndex( std::string() ),
    fastcgiParam( std::vector<std::string>() ),
    include( std::vector<std::string>() ),
    alias( std::string() ),
    redirect( std::string() )
{
}

Location::~Location()
{
}

/******************************
 * Setters
******************************/

/**
 * @brief Parse the URL's in a location block. The URL's are
 * the part between the "location" and "{" tokens.
 * @param tokens Vector with tokens
 * @throws std::runtime_error if the location block is invalid.
 */
void    Location::parseLocationURL( std::vector<std::string> &tokens )
{

    if ( tokens.size() == 1 || tokens[0] == "}" || tokens[0] == "{" ) 
        throw ( std::runtime_error( "Invalid Location block in config file" ) );
        
    /* Add the url to the location block. Also, remove any '/' at the end of 
    the url. This will make it easier to compare the URL's later. */
    this->url.push_back( std::regex_replace( tokens[0], std::regex("/+$"), "") );

    // Erase the current url from the tokens vector.
    tokens.erase( tokens.begin() );

    if ( tokens[0] != "{" ) 
        throw ( std::runtime_error( "Invalid Location block in config file" ) );
}

/**
 * @brief Parse an Allow instruction in a location block.
 * @param tokens Vector with tokens
 * @throws std::runtime_error if the instruction is invalid.
 */
void    Location::parseAllow( std::vector<std::string> &tokens )
{
    tokens.erase( tokens.begin() ); // Remove the 'allow' token.

    if ( tokens[0] == ";" || tokens.size() <= 1 || tokens[1] != ";" ) // Error check
            throw ( std::runtime_error( "Invalid 'Allow' instruction in config file" ) );

    if ( tokens[0] == "GET" )
        this->allowGet = true;
    else if ( tokens[0] == "POST" )
        this->allowPost = true;
    else if ( tokens[0] == "DELETE" )
        this->allowDelete = true;
    else
        throw ( std::runtime_error( "Error: Invalid argument for 'allow' instruction in location block." ) );

    tokens.erase( tokens.begin() ); // Remove the GET, POST or DELETE token.
    tokens.erase( tokens.begin() ); // Remove the ';' token.
}

/**
 * @brief Parse an autoindex instruction in a location block.
 * @param tokens Vector with tokens 
 */
void    Location::parseAutoindex ( std::vector<std::string> &tokens )
{
    tokens.erase( tokens.begin() ); // Remove the 'autoindex' token.

    if ( tokens[0] == ";" || tokens.size() <= 1 || tokens[1] != ";" ) // Error check
        throw ( std::runtime_error( "Invalid 'Autoindex' instruction in config file" ) );

    if ( tokens[0] == "on" )
        this->autoindex = true;
    else if ( tokens[0] == "off" )
        this->autoindex = false;
    else
        throw ( std::runtime_error( "Error: Invalid argument for 'Autoindex' instruction in location block." ) );

    tokens.erase( tokens.begin() ); // Remove the 'on' or 'off' token.
    tokens.erase( tokens.begin() ); // Remove the ';' token.
}

/**
 * @brief Parse alias instruction in a location block.
 * @param tokens Vector with tokens
 * @throw std::runtime_error if the instruction invalid.
 */
void    Location::parseAlias( std::vector<std::string> &tokens )
{
    tokens.erase( tokens.begin() ); // Remove the 'alias' token.

    if ( tokens[0] == ";" || tokens.size() <= 1 || tokens[1] != ";" ) // Error check
        throw ( std::runtime_error( "Invalid 'alias' instruction in config file" ) );

    this->alias = tokens[0]; // Set the alias.
    tokens.erase( tokens.begin() ); // Remove the alias token.
    tokens.erase( tokens.begin() ); // Remove the ';' token.
}

/**
 * @brief Parse fastcgi_pass instruction in a location block.
 * @param tokens Vector with tokens
 * @throw std::runtime_error if the instruction invalid.
 */
void    Location::parseFastcgiPass( std::vector<std::string> &tokens )
{
    tokens.erase( tokens.begin() ); // Remove the 'fastcgi_pass' token.

    if ( tokens[0] == ";" || tokens.size() <= 1 || tokens[1] != ";" ) // Error check
        throw ( std::runtime_error( "Invalid 'Fastcgi_pass' instruction in config file" ) );

    this->fastcgiPass = tokens[0]; // Set the fastcgi_pass.
    tokens.erase( tokens.begin() ); // Remove the fastcgi_pass token.
    tokens.erase( tokens.begin() ); // Remove the ';' token.
}

/**
 * @brief Parse fastcgi_index instruction in a location block.
 * @param tokens Vector with tokens
 * @throw std::runtime_error if the instruction invalid.
 */
void    Location::parseFastcgiIndex( std::vector<std::string> &tokens )
{
    tokens.erase( tokens.begin() ); // Remove the 'fastcgi_index' token.

    if ( tokens[0] == ";" || tokens.size() <= 1 || tokens[1] != ";" ) // Error check
        throw ( std::runtime_error( "Invalid Fastcgi_index instruction in config file" ) );

    this->fastcgiIndex = tokens[0]; // Set the fastcgi_index.
    tokens.erase( tokens.begin() ); // Remove the fastcgi_index token.
    tokens.erase( tokens.begin() ); // Remove the ';' token.
}

/**
 * @brief Parse fastcgi_param instruction in a location block.
 * @param tokens Vector with tokens
 * @throw std::runtime_error if the location block is invalid.
 */
void    Location::parseFastcgiParam( std::vector<std::string> &tokens)
{
    tokens.erase( tokens.begin() ); // Remove the 'fastcgi_param' token.

    while ( tokens[0] != ";" )
    {
        if ( tokens.size() <= 1 || tokens[0] == "}" ) 
            throw ( std::runtime_error( "Invalid Location block in config file" ) );
        this->fastcgiParam.push_back( tokens[0] );
        tokens.erase( tokens.begin() );
    }
    tokens.erase( tokens.begin() ); // Remove the ';' token.
}

/**
 * @brief Parse the include instruction in a location block.
 * @param tokens Vector with tokens
 * @throw std::runtime_error if the location block is invalid.
 */
void    Location::parseInclude( std::vector<std::string> &tokens )
{
    tokens.erase( tokens.begin() ); // Remove the 'include' token.

    while ( tokens[0] != ";" )
    {
        if ( tokens.size() == 1 || tokens[0] == "}" ) 
            throw ( std::runtime_error( "Invalid Location block in config file" ) );
        this->include.push_back( tokens[0] );
        tokens.erase( tokens.begin() );
    }
    tokens.erase( tokens.begin() ); // Remove the ';' token.
}

/**
 * @brief Parse the redirect instruction in a location block.
 * 
 */
void    Location::parseRedirect( std::vector<std::string> &tokens )
{
    tokens.erase( tokens.begin() ); // Remove the 'redirect' token.

    if ( tokens[0] == ";" || tokens.size() <= 1 || tokens[1] != ";" ) // Error check
        throw ( std::runtime_error( "Invalid 'redirect' instruction in config file" ) );

    this->redirect = tokens[0]; // Set the redirect.
    tokens.erase( tokens.begin() ); // Remove the redirect token.
    tokens.erase( tokens.begin() ); // Remove the ';' token.
}

/******************************
* Getters
*****************************/

/**
 * @brief Returns the URL's of the given location block.
 * @return std::vector std::string 
 */
std::vector<std::string> Location::getUrls( void ) const
{
    return ( this->url );
}

/**
 * @brief Returns a boolean indicating whether the given location block
 * has the GET method enabled.
 * @return true if GET is enabled, false otherwise.
 */
bool    Location::getAllowGet( void ) const
{
    return ( this->allowGet );
}

/**
 * @brief Returns a boolean indicating whether the given location block
 * has the autoindex enabled.
 * @return true if autoindex is enabled, false otherwise.
 */
bool    Location::getAutoindex( void ) const
{
    return ( this->autoindex );
}

/**
 * @brief Returns a boolean indicating whether the given location block
 * has the POST method enabled.
 * @return true if POST is enabled, false otherwise.
 */
bool    Location::getAllowPost( void ) const
{
    return ( this->allowPost );
}

/**
 * @brief Returns a boolean indicating whether the given location block
 * has the DELETE method enabled.
 * @return true if DELETE is enabled, false otherwise.
 */
bool    Location::getAllowDelete( void ) const
{
    return ( this->allowDelete );
}

/**
 * @brief Returns the FastCGI pass of the given location block.
 * @return std::string 
 */
std::string Location::getFastcgiPass( void ) const
{
    return ( this->fastcgiPass );
}

/**
 * @brief Returns the FastCGI Index of the given location block.
 * @return std::string 
 */
std::string Location::getFastcgiIndex( void ) const
{
    return ( this->fastcgiIndex );
}

/**
 * @brief Returns the FastCGI param of the given location block.
 * @return std::vector std::string 
 */
std::vector<std::string> Location::getFastcgiParam( void ) const
{
    return ( this->fastcgiParam );
}

/**
 * @brief Returns the includes of the given location block.
 * @return std::vector std::string 
 */
std::vector<std::string> Location::getInclude( void ) const
{
    return ( this->include );
}

/**
 * @brief Returns the alias of the given location block.
 * @return std::string 
 */
std::string Location::getAlias( void ) const
{
    return ( this->alias );
}

/**
 * @brief Returns the redirect of the given location block.
 */
std::string Location::getRedirect( void ) const
{
    return ( this->redirect );
}
