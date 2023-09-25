#include <HttpRequest.hpp>
#include <HttpResponse.hpp>
#include <Server.hpp>

/**
 * @brief Check if request line has the right syntax and save the method & URI
 * 
 * @param line  request line
 */
void		HttpRequest::isRequestLine(std::string line, HttpResponse &response)
{
    std::string s;
    std::stringstream ss(line);
    std::vector<std::string> v; // v[0] = method, v[1] = request-target, v[2] = HTTP-version
    while (getline(ss, s, ' ')) {
        // store token string in the vector
        v.push_back(s);
    }
    if ((int)v.size() != 3)
        return (response.setError(400, "Bad Request"));
    if (v[0] == "GET")
        setMethod(GET);
    else if (v[0] == "POST")
        setMethod(POST);
    else if (v[0] == "DELETE")
        setMethod(DELETE);
    else
        return (response.setError(501, "Not Implemented"));
    if (v[1].empty())
        return (response.setError(400, "Bad Request"));
    setURI(v[1]);
    v[2].erase(std::remove(v[2].begin(), v[2].end(), '\r'), v[2].end());
    if (v[2] != "HTTP/1.1")
        return (response.setError(505, "HTTP Version Not Supported"));
}

/**
 * @brief Check if header has the right syntax and save the header field
 * 
 * @param line header line
 */
void		HttpRequest::isHeader(std::string line, HttpResponse &response)
{
    std::string s;
    std::stringstream ss(line);
    std::vector<std::string> v; // v[0] = field-name, v[1] = field-value
    while (getline(ss, s, ':')) {
        // store token string in the vector
        v.push_back(s);
    }
    // if ((int)v.size() != 2)
    //     throw ( std::runtime_error( "wrong header field" ) );
    if (std::isspace(v[0].back()))
        return (response.setError(400, "Bad Request"));
    v[0].erase(std::remove(v[0].begin(), v[0].end(), ' '), v[0].end()); // remove whitespace
    v[1].erase(std::remove(v[1].begin(), v[1].end(), ' '), v[1].end()); // remove whitespace
    v[1].erase(std::remove(v[1].begin(), v[1].end(), '\r'), v[1].end()); // remove \r
    if (v[0] == "Content-Type")
        setContentType(v[1]);
    if (v[0] == "Host")
        setHost(v[1]);
}

/**
 * @brief Parse the request and save the information
 * 
 * @param buffer  request
 */
std::string    HttpRequest::parseRequestandGiveReponse(std::vector<char> buffer, Server server)
{
	std::string file(buffer.begin(), buffer.end());
    std::stringstream ss(file);

    std::string line;
    HttpResponse response;
    bool emptyLineFound = false;
    while (std::getline(ss, line)) // Use newline '\n' as the delimiter
    {
        std::cout << "line " << line;
        if (!line.find("GET") || !line.find("POST") || !line.find("DELETE")) // request line
            isRequestLine(line, response);
        else if (line == "\r" || line == "") // empty line (i.e., a line with nothing preceding the CRLF)
            emptyLineFound = true;
        else if(emptyLineFound == false) // header line
            isHeader(line, response);
        else // body line
            addLineToBody(line);
    }
    // printAll();
    if (_request_URI.size() >= 3 && _request_URI.substr(_request_URI.size() - 3, 3) == ".py")
    {
        parseCgiRequest(response);
    }
    return (response.buildResponse(server));
}
