#include "HttpServer.hpp" 
/*
To expand on this example, a user wants to visit TechTarget.com.
The user types in the web address and the computer sends a "GET" request to a server that hosts that address.
That GET request is sent using HTTP and tells the TechTarget server that the user is looking for the HTML 
(Hypertext Markup Language) code used to structure and give the login page its look and feel. The text of that
login page is included in the HTML response, but other parts of the page -- particularly its images and videos --
are requested by separate HTTP requests and responses. The more requests that are made -- for example, to call a 
page that has numerous images -- the longer it will take the server to respond to
those requests and for the user's system to load the page.
*/


/*
Receiving the Request: The server listens on a specific port for incoming HTTP requests. When a request arrives, the server's networking layer reads the request from the socket and stores it in memory for further processing.

Splitting the Request: The server first separates the request into three main parts: the request line, the headers, and the message body. These parts are typically divided by line breaks ("\r\n" or "\n").

Parsing the Request Line: The server parses the request line to extract the following information:

HTTP Method: The HTTP method (GET, POST, PUT, DELETE, etc.) indicates the type of operation the client wants to perform.
Requested URI: The Uniform Resource Identifier (URI) specifies the target resource the client is requesting.
HTTP Version: The version of the HTTP protocol being used (e.g., HTTP/1.1, HTTP/2).
Parsing Headers: The server then parses the headers section, which contains various headers in key-value format. It extracts information such as the content type, content length, user-agent, authentication tokens, and more. The server may use this information to process the request appropriately.

Handling Message Body (For Appropriate Methods): For certain HTTP methods (e.g., POST, PUT), the request may have a message body containing data sent by the client (e.g., form data, JSON). The server extracts and processes this data based on the specified content type in the headers.

Performing Action: After parsing the entire request, the server determines the appropriate action to take based on the HTTP method, the requested URI, and the provided data (if any). It then processes the request, which might involve accessing databases, handling business logic, or generating a response.

Generating Response: Once the server processes the request and generates the desired response (e.g., HTML, JSON, XML), it creates an HTTP response with a status code, headers, and the response body. The server sends this response back to the client over the network.
*/

/**
 * @brief Check if this is the request line and save the method
 * 
 * @param line 
 */
//  A server that receives a method longer than any that it implements SHOULD respond with a 501 (Not Implemented) 
//  status code. A server that receives a request-target longer than any URI it wishes to parse MUST respond with a 
//  414 (URI Too Long) status code (see Section 15.5.15 of [HTTP]).
// Recipients of an invalid request-line SHOULD respond with either a 400 (Bad Request) error or a 301 (Moved Permanently)
//  redirect with the request-target properly encoded.
void		HttpServer::isRequestLine(std::string line)
{
    if (line.find("GET") && line.find("POST") && line.find("DELETE"))
        return ;
    // variable to store token obtained from the original string
    std::string s;
    // constructing stream from the string
    std::stringstream ss(line);
    // declaring vector to store the string after split
    std::vector<std::string> v;
    while (getline(ss, s, ' ')) {
        // store token string in the vector
        v.push_back(s);
    }
    if ((int)v.size() != 3)
        throw ( std::runtime_error( "wrong request line" ) );
    if (v[0] == "GET")
        setMethod(GET);
    else if (v[0] == "POST")
        setMethod(POST);
    else if (v[0] == "DELETE")
        setMethod(DELETE);
    else
        throw ( std::runtime_error( "wrong method" ) );
    setURI(v[1]);
    if (v[2] != "HTTP/1.1")
        throw ( std::runtime_error( "HTTP 1.1 not given" ) );
}


/*
A Request-line  HTTP 1.1

Zero or more header (General|Request|Entity) fields followed by CRLF

An empty line (i.e., a line with nothing preceding the CRLF) 
indicating the end of the header fields

Optionally a message-body
*/

/**
*/
void    HttpServer::parseRequest(std::vector<char> buffer)
{
	std::string file(buffer.begin(), buffer.end());
    std::stringstream ss(file);

    /* parse per line */
    std::string line;
    while (std::getline(ss, line)) // Use newline '\n' as the delimiter
    {
        // Process each line here, or store it in a container for further processing
		isRequestLine(line);
        // isHeader(line);
        
        // For example, you can print the line to the console:
        std::cout << line << std::endl;
    }
}
// A recipient that receives whitespace between the start-line and the first header field MUST either reject the
//  message as invalid or consume each whitespace-preceded line without further processing of it (i.e., ignore the entire 
//  line, along with any subsequent lines preceded by whitespace, until a properly formed header field is received or the
//   header section is terminated). Rejection or removal of invalid whitespace-preceded lines is necessary to prevent their 
//   misinterpretation by downstream recipients that might be vulnerable to request smuggling (Section 11.2) or response 
//   splitting (Section 11.1) attacks.

// When a server listening only for HTTP request messages, or processing what appears from the start-line to be an HTTP 
// request message, receives a sequence of octets that does not match the HTTP-message grammar aside from the robustness
//  exceptions listed above, the server SHOULD respond with a 400 (Bad Request) response and close the connection.

// A server MUST respond with a 400 (Bad Request) status code to any HTTP/1.1 request message that lacks a Host header 
// field and to any request message that contains more than one Host header field line or a Host header field with an 
// invalid field value.

// A server that receives a request header field line, field value, or set of fields larger than it wishes to process MUST 
// respond with an appropriate 4xx (Client Error) status code. Ignoring such header fields would increase the server's 
// vulnerability to request smuggling attacks (Section 11.2 of [HTTP/1.1]).