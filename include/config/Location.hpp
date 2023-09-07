/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Location.hpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: cpost <cpost@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/07/24 12:19:54 by cpost         #+#    #+#                 */
/*   Updated: 2023/09/07 13:16:14 by cpost         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <string>
# include <vector>

class Location
{
    private:

        std::vector<std::string>		url;
	    bool							allowGet;
	    bool							allowPost;
	    bool							allowDelete;
		bool							autoindex;
	    std::string						fastcgiPass;
	    std::string						fastcgiIndex;
	    std::vector<std::string>		fastcgiParam;
	    std::vector<std::string>		include;
    
    public:
        
    /*****************************
	* Constructors / Destructor
	****************************/
        
        Location( void );
        ~Location();

    /******************************
	* Setters
	*****************************/

        void    parseLocationURL( std::vector<std::string> &tokens );
		void	parseAllow( std::vector<std::string> &tokens );
		void	parseAutoindex( std::vector<std::string> &tokens );
		void	parseFastcgiPass( std::vector<std::string> &tokens );
		void	parseFastcgiIndex( std::vector<std::string> &tokens );
		void	parseFastcgiParam( std::vector<std::string> &tokens );
		void	parseInclude( std::vector<std::string> &tokens);

    /******************************
	* Getters
	*****************************/

		bool						getAllowGet( void ) const;
		bool						getAllowPost( void ) const;
		bool						getAllowDelete( void ) const;
		bool						getAutoindex( void ) const;
        std::vector<std::string>	getUrls( void ) const;
		std::string					getFastcgiPass( void ) const;
		std::string					getFastcgiIndex( void ) const;
		std::vector<std::string>	getFastcgiParam( void ) const;
		std::vector<std::string>	getInclude( void ) const;
};

#endif