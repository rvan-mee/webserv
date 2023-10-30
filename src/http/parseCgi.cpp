#include <HttpRequest.hpp>
#include <Server.hpp>
#include <EventPoll.hpp>

void HttpRequest::parseCgiRequest(HttpResponse &response, Server server, bool& isCgiRequest, std::string request)
{
    if (_request_method == 0 || _request_method == 1)
    {
        size_t slashPos = _request_URI.find_last_of('/');

        // Find the position of '.py' starting from the position of the last '/'
        size_t dotPyPos = _request_URI.find(".py", slashPos);

        // Extract the substring between the last '/' and '.py'
        std::string result = _request_URI.substr(slashPos + 1, dotPyPos - slashPos - 1);

        // std::cout << "Result: " << result << std::endl; 
        std::ifstream file(server.getLocation(".py").getAlias() + result + ".py");

        // Check if the file is open
        if (file.is_open()) {
            // You can now read or manipulate the file here if needed.
            try {

                // _cgi.setWriteBuffer(); // TODO: if you want to give the CGI some info to work with 
                _cgi.startPythonCgi(server.getLocation(".py").getAlias() + result + ".py", request);
                isCgiRequest = true;
                
            }
            catch (std::exception &e)
            {
                std::cerr << e.what() << std::endl;
                file.close();
                return (response.setError(500, "Internal server error"));
            }
            file.close(); // Don't forget to close the file when you're done with it.
            return ;
        } else {
            return (response.setError(404, "Not Found"));
        }
    }
    else
        return (response.setError(405, "Method Not Allowed"));
}
