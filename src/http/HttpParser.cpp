#include <HttpRequest.hpp>
#include <HttpResponse.hpp>
#include <Server.hpp>
#include <sys/stat.h>
#include <poll.h>
#include <CgiHandler.hpp>
#include <EventPoll.hpp>
#include <cstdio>

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
    try {
        if (pathExists(server.getRoot() +_request_URI) && server.getLocation(_request_URI).getAllowGet() == 0)
        {
            return (response.setError(405, "Method Not Allowed"));
        }
    }
    catch (std::exception &e){ 
    }
    if ( _request_URI == "/redirect" )
    {
        response.setRedirect( server.getRedirect() );
        return (response.setError( 301, "Moved Permanently" ));
    }
    //GET /favicon.ico HTTP/1.1
    else if (_request_URI != "/" && isDirectory(server.getRoot() + _request_URI)) {
        if (server.getAutoindex() == false)
                return (response.setError(403, "Forbidden"));
            else
                return (response.buildBodyDirectory(server.getRoot() + _request_URI, server));
    }
    else if (_request_URI != "/")
        return (response.buildBodyFile(server.getRoot() + _request_URI));
    response.buildResponse(server);
}


void		HttpRequest::parsePostRequest(HttpResponse &response, Server server, bool& isCgiRequest, std::string request)
{
    try {
        if (pathExists(server.getRoot() +_request_URI) && server.getLocation(_request_URI).getAllowPost() == false)
        {
            return (response.setError(405, "Method Not Allowed"));
        }
    }
    catch (std::exception &e){
    }
    //in case of file upload
    // std::cout << "content type: " << _content_type << std::endl;
    if (_content_type == "multipart/form-data") {
        // You can now read or manipulate the file here if needed.
        try {
            //give body input to python script
            _cgi.startPythonCgi(server.getLocation(".py").getAlias() + "upload.py", request);
            isCgiRequest = true;
        }
        catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            return (response.setError(505, "Internal Server Error"));
        }
        return ;
    }
    return (response.setError(415, "Unsupported Media Type"));
    //moeten we deze error wel geven? bvb bij json
}

void		HttpRequest::parseDeleteRequest(HttpResponse &response, Server server)
{
    if (!pathExists(server.getRoot() + _request_URI)) {
        _poll.addEvent(_socketFd, POLLOUT);
        return (response.setError(204, "No Content"));
    }
     try {
        if (pathExists(server.getRoot() +_request_URI) && server.getLocation(_request_URI).getAllowDelete() == false)
        {
            _poll.addEvent(_socketFd, POLLOUT);
            return (response.setError(405, "Method Not Allowed"));
        }
    }
    catch (std::exception &e){
    }
    if (std::remove((server.getRoot() + _request_URI).c_str()) == 0) {
        std::cout << "File deleted successfully: " << _request_URI << std::endl;
    } else {
        return (response.setError(500, "Internal Server Error"));
    }
}
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
void		HttpRequest::isHeader(std::string line, HttpResponse &response, Config config)
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
    {
        try
        {
            _host = config.getServer(v[1]).getServerNames()[0]; //if server isn't found, default server will be the host and a 404 response will be returned
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            return (response.setError(400, "Bad Request"));
        }
    }
}

/**
 * @brief Parse the request and save the information
 * 
 * @param buffer  request
 */
std::string    HttpRequest::parseRequestAndGiveResponse(std::vector<char> buffer, Config config)
{
	std::string file(buffer.begin(), buffer.end());
    std::stringstream ss(file);

    std::string line;
    HttpResponse response;
    bool    emptyLineFound = false;
    bool    isCgiRequest = false;
    std::string request;
	std::cout << "Request:" << std::endl;

    while (std::getline(ss, line)) // Use newline '\n' as the delimiter
    {
        std::cout << line << std::endl;
        request += line;
        request += "\n";
        if (!line.find("GET") || !line.find("POST") || !line.find("DELETE")) // request line
            isRequestLine(line, response);
        else if (line == "\r" || line == "") // empty line (i.e., a line with nothing preceding the CRLF)
            emptyLineFound = true;
        else if(emptyLineFound == false) // header line
            isHeader(line, response, config);
        else // body line
            addLineToBody(line);
    }

    // printAll();
    if (_request_URI.size() >= 3 && _request_URI.substr(_request_URI.size() - 3, 3) == ".py") {
        parseCgiRequest(response, config.getServer(_host), isCgiRequest, request);
    }
    else if (_request_method == GET && (_request_URI == "/redirect" || pathExists(config.getServer(_host).getRoot() +_request_URI))) {
        parseGetRequest(response, config.getServer(_host));
    } 
    else if (_request_method == POST && pathExists(config.getServer(_host).getRoot() +_request_URI)) {
        parsePostRequest(response, config.getServer(_host), isCgiRequest, request);
    }
    else if (_request_method == DELETE) {
        parseDeleteRequest(response, config.getServer(_host));
    }
    else if (_request_method == GET || _request_method == POST) {
        // Handle non-existent path
        response.setError(404, "Not Found");
    }

    // if the request is not for the CGI we can write
    // the response to the socket instead of waiting for the CGi to finish.
    if (!isCgiRequest)
        _poll.addEvent(_socketFd, POLLOUT);
    return (response.buildResponse(config.getServer(_host)));
}
