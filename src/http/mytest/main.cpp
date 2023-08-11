#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
//g++ -Wall -Wextra -Werror main.cpp HttpParser.cpp HttpResponse.cpp HttpRequest.cpp && ./a.out 
// curl -X POST -H "Content-Type: plain/text" --data "BODY IS HERE write something shorter or longer than body limit"
int main()
{
    HttpRequest server;
    std::string s = "POST /cgi-bin/process.cgiHTTP/1.1\r\n User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)\nHost: www.tutorialspoint.com\nContent-Type: application/x-www-form-urlencoded\nContent-Length: length\nAccept-Language: en-us\nAccept-Encoding: gzip, deflate\nConnection: Keep-Alive\n\nlicenseID=string&content=string&/paramsXML=string";
    std::vector<char> v;
    std::copy(s.begin(), s.end(), std::back_inserter(v));
    try {
        std::cout << server.parseRequestandGiveReponse(v);
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return (1);
    }
    return (0);
}
