#include "HttpServer.hpp" 

int main()
{
    HttpServer server;
    std::string s = "POST /cgi-bin/process.cgi HTTP/1.1\nUser-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)\nHost: www.tutorialspoint.com\nContent-Type: application/x-www-form-urlencoded\nContent-Length: length\nAccept-Language: en-us\nAccept-Encoding: gzip, deflate\nConnection: Keep-Alive\n\nlicenseID=string&content=string&/paramsXML=string";
	// std::vector<char> buffer(s.begin(), s.end());
    std::vector<char> v;
    std::copy(s.begin(), s.end(), std::back_inserter(v));
    server.parseRequest(buffer);
}
