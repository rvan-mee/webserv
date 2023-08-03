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
void    HttpServer::parseRequest(std::vector<char> buffer)
{
	// std::string file(buffer.begin(), buffer.end());
    std::stringstream ss(buffer.begin(), buffer.end());

	/* parse per line */
	std::string line;
	while ( std::getline( ss, line, ' ' ) ) 
		line = '1';
}
