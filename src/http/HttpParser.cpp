#include <HttpRequest.hpp>
#include <HttpResponse.hpp>
#include <Server.hpp>
#include <sys/stat.h>
#include <poll.h>

bool pathExists(const std::string& path) {
    struct stat info;
    return stat(path.c_str(), &info) == 0;
}

bool isDirectory(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        return false; // Path does not exist
    }
    return (info.st_mode & S_IFDIR) != 0; // Check if it's a directory
}


void		HttpRequest::parseGetRequest(HttpResponse &response, Server server)
{
    //GET /favicon.ico HTTP/1.1
    if (_request_URI != "/" && isDirectory(server.getRoot() + _request_URI)) {
        if (server.getAutoindex() == false)
                return (response.setError(403, "Forbidden"));
            else
                return (response.buildBodyDirectory(server.getRoot() + _request_URI));
    }
    else if (_request_URI.find(server.getUploadsDir()) != std::string::npos) //check if get request is file
        return (response.buildBodyFile(_request_URI.substr(_request_URI.find_last_of('/') + 1)));
    else
        return (response.setBodyHtml(server.getRoot() + _request_URI));
}

/**
 * @brief Check if request line has the right syntax and save the method & URI
 * 
 * @param line  request line
 */
void		HttpRequest::isRequestLine(std::string line, HttpResponse &response, Server server)
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
    std::cout << server.getRoot() + v[1] << std::endl;
    if (_request_method == GET && pathExists(server.getRoot() + v[1])) {
        parseGetRequest(response, server);
    } else if (_request_method == GET) {
        // Handle non-existent path
        return (response.setError(400, "Bad Request"));
    }
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
    if ((int)v.size() < 2)
        throw ( std::runtime_error( "wrong header field" ) );
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
std::string    HttpRequest::parseRequestAndGiveResponse(std::vector<char> buffer, Server server, EventPoll& poll)
{
	std::string file(buffer.begin(), buffer.end());
    std::stringstream ss(file);

    std::string line;
    HttpResponse response;
    bool emptyLineFound = false;
	// std::cout << "Request:" << std::endl;
    while (std::getline(ss, line)) // Use newline '\n' as the delimiter
    {
        // std::cout << line << std::endl;
        if (!line.find("GET") || !line.find("POST") || !line.find("DELETE")) // request line
            isRequestLine(line, response, server);
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
        parseCgiRequest(response, server, poll);
    }
    //     return (response.buildResponse(server));
    // else if (_request_method == POST)
    //     return (response.buildResponse(server));
    // else if (_request_method == DELETE)
    return (response.buildResponse(server));
}
