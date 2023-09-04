#include <HttpRequest.hpp>

void HttpRequest::parseCgiRequest(HttpResponse &response)
{
    if (_request_method == 0 || _request_method == 1)
    {
        //check if script exists
        //if not 
          // Create an input stream object
            //   std::string input = "/cgi-bin/some_script.cgi";
        std::string delimiter = "/cgi-bin/";

        size_t pos = _request_URI.find(delimiter); // Find the position of the delimiter
        std::string result;
        if (pos != std::string::npos) {
            result = _request_URI.substr(pos + delimiter.length()); // Extract the part after the delimiter
            std::cout << "Result: " << result << std::endl;
        } else {
            std::cout << "Delimiter not found in the input string." << std::endl;
        }
        std::ifstream file("/Users/dkramer/WEBSERVfolder/cgi-bin/" + result);

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
