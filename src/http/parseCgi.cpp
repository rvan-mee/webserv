#include <HttpRequest.hpp>

void HttpRequest::parseCgiRequest()
{
    if (_request_method == 0 || _request_method == 1)
    {
        //check if script exists
        //if not 
        return (response.setError(404, "The requested resource was not found"));
    }
    else
        return (response.setError(405, "Method Not Allowed"));
}
