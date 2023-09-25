#include <HttpRequest.hpp>

void HttpRequest::parseCgiRequest(HttpResponse &response)
{
    if (_request_method == 0 || _request_method == 1)
    {
        size_t slashPos = _request_URI.find_last_of('/');

        // Find the position of '.py' starting from the position of the last '/'
        size_t dotPyPos = _request_URI.find(".py", slashPos);

        // Extract the substring between the last '/' and '.py'
        std::string result = _request_URI.substr(slashPos + 1, dotPyPos - slashPos - 1);

        std::cout << "Result: " << result << std::endl; 
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
