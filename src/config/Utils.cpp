/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   utils.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: cpost <cpost@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/07/03 18:09:55 by cpost         #+#    #+#                 */
/*   Updated: 2023/07/24 15:47:39 by cpost         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>

/**
 * @brief Function to read a file to a stringstream.
 * @param filePath Path to the file
 * @return const std::string. String with the file contents in it.
 */
std::string readFile( const std::string& filePath ) 
{
    std::ifstream inputFile( filePath ); // Open file
    std::stringstream ss; // stringstream to temporarly store the file contents

    if ( inputFile ) 
    {
        ss << inputFile.rdbuf(); // Save file contents to stringstream
        inputFile.close(); // Close file
    }
    return ( ss.str() ); /* Return string with file contents in it. 
    (If the file could not be read, the string will be empty). */
}

/**
 * @brief Function that trims the text after a '/' in a URL. 
 * If there is no '/' in the URL, the function returns an empty string.
 * @param input 
 * @return true if the URL was trimmed, false if not.
 */
bool trimLocationUrl( std::string& input ) 
{
    if ( input == "/" ) // If the URL is just a '/'.
    {
        input = "";
        return ( false );
    }

    size_t lastSlashPos = input.find_last_of('/');
    if (lastSlashPos != std::string::npos) {
        input = input.substr(0, lastSlashPos);
        return ( true );
    }
    return ( false );
}

/**
 * @brief Function to remove unnecessary things from a line.
 * @param line The line to trim.
 */
void    trimLine( std::string &line )
{
    // Remove all unnecessary spaces from the line.
    line = std::regex_replace(line, std::regex("^ +| +$|( ) +"), "$1");
    // Empty line if it starts with a newline.
    line = std::regex_replace(line, std::regex("^\\n"), "");
    // Add a space before a semicolon if there is none. This is needed for tokenizing.
    line = std::regex_replace(line, std::regex("([^\\s;])\\s*;"), "$1 ;");
    // Empyty line if it starts with a #.
    line = std::regex_replace(line, std::regex("^#.*$"), "");
}
