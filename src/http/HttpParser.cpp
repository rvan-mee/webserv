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
                return (response.buildBodyDirectory(_request_URI, server));
    }
    else if (_request_URI != "/")
        return (response.buildBodyFile(server.getRoot() + _request_URI));
    response.buildResponse(server);
}

std::string extractFilenameFromHTTPRequest(const std::string& httpRequest) {
    // Find the position of "Content-Disposition" in the HTTP request.
    std::size_t pos = httpRequest.find("Content-Disposition");
    if (pos == std::string::npos) {
        return ""; // Header not found, or it's not a POST request.
    }

    // Find the position of "filename=" in the "Content-Disposition" header.
    pos = httpRequest.find("filename=", pos);
    if (pos == std::string::npos) {
        return ""; // "filename=" not found in the header.
    }

    // Extract the filename from the header.
    pos += 10; // Move past "filename=".
    std::size_t endPos = httpRequest.find("\"", pos);
    if (endPos == std::string::npos) {
        return ""; // Filename closing quote not found.
    }

    // Extract and return the filename.
    std::string filename = httpRequest.substr(pos, endPos - pos);
    return filename;
}

std::string extractContent(const std::string &inputText, const std::string &boundary) {
  std::istringstream input(inputText);
    std::string line;
    std::ostringstream extractedContent;

    while (std::getline(input, line)) {
        if (line.find("Content-Type:") != std::string::npos)
        {

            while (std::getline(input, line))
            {
                if (line.find(boundary) != std::string::npos)
                  break;
                extractedContent << line << "\n";
            }
        }
    }
    return extractedContent.str();
}


void		HttpRequest::parsePostRequest(HttpResponse &response, Server server, std::string request)
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
    if (_content_type.find("multipart/form-data") != std::string::npos) { 
        try {
            std::string fileName = extractFilenameFromHTTPRequest(request);
            std::string filePath = server.getUploadsDir() + fileName;
            std::cout << "filepaht: " << filePath << std::endl;
            std::ofstream uploadFile(filePath);
            std::string content = extractContent(_message_body, "------");
            if (uploadFile.is_open()) {
                // uploadFile.write(content.c_str(), content.size());
                uploadFile << content;
                uploadFile.close();
                response.setMessageBodyText("File saved: " + filePath);
                response.buildResponse(server);
            } else
                response.setError(400, "Bad Request");
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
        return (response.setError(204, "No Content"));
    }
    try {
        if (server.getLocation(_request_URI).getAllowDelete() == false)
        {
            return (response.setError(405, "Method Not Allowed"));
        }
    }
    catch (std::exception &e){

    }
    if (std::remove((server.getRoot() + _request_URI).c_str()) == 0) {
        response.setMessageBodyText("File deleted: " + _request_URI);
        response.buildResponse(server);
    }
    else
        return (response.setError(500, "Internal Server Error"));
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
void		HttpRequest::isHeader(std::string line, HttpResponse &response, Config config, int port)
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
        try {
            //if server isn't found, default server will be the host and a 404 response will be returned
            _host = config.getServer(v[1], port).getServerNames()[0];
        }
        catch(const std::exception& e) {
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
std::string    HttpRequest::parseRequestAndGiveResponse(std::vector<char> buffer, Config config, int port)
{
	std::string file(buffer.begin(), buffer.end());
    std::stringstream ss(file);

    std::string line;
    HttpResponse response;
    bool    emptyLineFound = false;
    bool    isCgiRequest = false;
    std::string request;

    while (std::getline(ss, line)) // Use newline '\n' as the delimiter
    {
        request += line;
        request += "\n";
        if (line.find("HTTP/1.1") != std::string::npos) // request line
            isRequestLine(line, response);
        else if (line == "\r" || line == "") // empty line (i.e., a line with nothing preceding the CRLF)
            emptyLineFound = true;
        else if(emptyLineFound == false) // header line
            isHeader(line, response, config, port);
        else // body line
            addLineToBody(line);
    }

    // printAll();
    if (_request_URI.size() >= 3 && _request_URI.substr(_request_URI.size() - 3, 3) == ".py") {
        parseCgiRequest(response, config.getServer(_host, port), isCgiRequest, request);
    }
    else if (_request_method == GET && (_request_URI == "/redirect" || pathExists(config.getServer(_host, port).getRoot() +_request_URI))) {
        parseGetRequest(response, config.getServer(_host, port));
    } 
    else if (_request_method == POST && pathExists(config.getServer(_host, port).getRoot() +_request_URI)) {
        parsePostRequest(response, config.getServer(_host, port), request);
    }
    else if (_request_method == DELETE) {
        parseDeleteRequest(response, config.getServer(_host, port));
    }
    else if (_request_method == GET || _request_method == POST) {
        // Handle non-existent path
        response.setError(404, "Not Found");
    }

    // if the request is not for the CGI we can write
    // the response to the socket instead of waiting for the CGi to finish.
    if (!isCgiRequest)
        _poll.addEvent(_socketFd, POLLOUT);
    return (response.buildResponse(config.getServer(_host, port)));
}
