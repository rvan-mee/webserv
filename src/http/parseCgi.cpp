#include <HttpRequest.hpp>

void HttpRequest::parseCgiRequest(HttpResponse &response)
{
    if (_request_method == 0 || _request_method == 1)
    {
        std::cout << "CGI REQUEST" << std::endl;
        //check if script exists
        //if not 
          // Create an input stream object
            //   std::string input = "/cgi-bin/some_script.cgi";
        // std::string delimiter = "/cgi-bin/";

        // size_t pos = _request_URI.find(delimiter); // Find the position of the delimiter
        // std::string result;
        // if (pos != std::string::npos) {
    //     if (!ft_strrchr(game->mapdata.map_path, '.') || \
	// ft_strncmp(ft_strrchr(game->mapdata.map_path, '.'), ".cub", 5))
	// 	return (msg_err_exit("Please provide a map with .cub extension.", 1));

    //        size_t found = _request_URI.rfind('.');
    
    // if (found != std::string::npos) {

    //     if (strncmp(_request_URI.substr(found + 6), ".py", 4) == 0)
    //         std::cout << "Last occurrence of '" << ".py" << "' found at position: " << found << std::endl;
    // } else {
    //     std::cout << ".py not found in the string." << std::endl;
    // }
    //  if (_request_URI.find_last_of('.') == std::string::npos || 
    //     _request_URI.substr(_request_URI.find_last_of('.')) != ".py") {
    //     // Return an error message or handle the error as needed
    //     std::cout << "Please provide a file with .py extension." << std::endl;
    //     return ;
    // }

   size_t slashPos = _request_URI.find_last_of('/');

    // Find the position of '.py' starting from the position of the last '/'
    size_t dotPyPos = _request_URI.find(".py", slashPos);

        // Extract the substring between the last '/' and '.py'
        std::string result = _request_URI.substr(slashPos + 1, dotPyPos - slashPos - 1);

        std::cout << "Result: " << result << std::endl; 
        // } else {
        //     std::cout << "Delimiter not found in the input string." << std::endl;
        // }
        std::ifstream file("/Users/dkramer/WEBSERVfolder/cgi-bin/" + result + ".py");

        // Check if the file is open
        if (file.is_open()) {
            std::cout << "File exists." << std::endl;
            // You can now read or manipulate the file here if needed.
            file.close(); // Don't forget to close the file when you're done with it.
        } else {
            return (response.setError(404, "The requested resource was not found"));
        }
    }
    else
        return (response.setError(405, "Method Not Allowed"));
}
