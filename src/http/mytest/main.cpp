#include "HttpServer.hpp" 
//g++ -Wall -Wextra -Werror main.cpp HttpParser.cpp HttpServer.cpp && ./a.out
// curl -X POST -H "Content-Type: plain/text" --data "BODY IS HERE write something shorter or longer than body limit"
int main()
{
    HttpServer server;
    std::string s = "POST /cgi-bin/process.cgi HTTP/1.1\r\nUser-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)\nHost: www.tutorialspoint.com\nContent-Type: application/x-www-form-urlencoded\nContent-Length: length\nAccept-Language: en-us\nAccept-Encoding: gzip, deflate\nConnection: Keep-Alive\n\nlicenseID=string&content=string&/paramsXML=string";
    std::vector<char> v;
    std::copy(s.begin(), s.end(), std::back_inserter(v));
    try {
        server.parseRequest(v);
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return (1);
    }
    return (0);
}
