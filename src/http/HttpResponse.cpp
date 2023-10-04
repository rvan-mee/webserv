#include <HttpResponse.hpp>
#include <Server.hpp>
#include <dirent.h>
#include <sys/stat.h>

HttpResponse::HttpResponse()
{
	_status_code = 0;
	_reason_phrase = "";
	_content_type = "";
	_message_body = "";
}

void HttpResponse::setContentType(std::string contentType)
{
	// if (contentType.empty())
	// 	throw(std::runtime_error("no content-type in response found"));
	_content_type = contentType;
}

void HttpResponse::addLineToBody(std::string line)
{
	_message_body += line;
	_message_body += "\n";
}

void HttpResponse::printAll()
{
	std::cout << "Reason Phrase: " << _reason_phrase << std::endl;
	std::cout << "status code: " << _status_code << std::endl;
	std::cout << "Content-Type: " << _content_type << std::endl;
	std::cout << "Body: " << _message_body << std::endl;
}

void HttpResponse::setError(int statusCode, std::string reasonPhrase)
{
	_status_code = statusCode;
	_reason_phrase = reasonPhrase;
}

bool replace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

void HttpResponse::buildBodyDirectory(std::string directoryPath, Server server)
{
	//users must be able to input any
	if (!server.getIndex().empty())
		std::cout << "index: " << server.getIndex()[0] << std::endl;
	if (!server.getIndex().empty() && !server.getIndex()[0].empty())
	{
		//check if index file exists
		std::ifstream index(server.getIndex()[0]); //taking file as inputstream
		std::string e;
		if (index.is_open())
		{
			std::ostringstream ss;
			ss << index.rdbuf(); // reading data
			e = ss.str();
			_message_body = e;
		}
		else
		{
			std::cout << "Error opening file";

		}
		//if it does, buildBodyFile(indexFile)
		//else, buildBodyDirectory(directoryPath)
	}
	else
	{
		_message_body += "<html>\n<head><title>Index of " + directoryPath + "</title></head>\n";
		_message_body += "<h1>Index of " + directoryPath + "</h1><hr>\n";

		DIR *dir;
		struct dirent *ent;
		if ((dir = opendir (directoryPath.c_str())) == NULL)
		{
			std::cerr << "Can't open directory: " << directoryPath << std::endl;
		}
		_message_body += "<table><tr><th>Name</th><th>Last Modified</th><th>Size (bytes)</th></tr>";
		while ((ent = readdir(dir)) != NULL)
		{
			if (std::string(ent->d_name) == ".")
				continue ;

			std::string filepath = directoryPath;
			if(directoryPath.back()!= '/'){
				filepath += '/';
			}
			filepath += ent->d_name;

			//get directory/file information
			struct stat buf; //struct to store file/directory data
			stat(filepath.c_str(), &buf);
		

			_message_body += "<tr><td>";
			_message_body += "<a href=\"";
			_message_body += ent->d_name;
			if (ent->d_type == DT_DIR)
				_message_body += "/";
			_message_body += "\">";
			_message_body += ent->d_name;
			if (ent->d_type == DT_DIR)
				_message_body += "/";
			_message_body += "</a></td><td>";
			if (ent->d_type != DT_DIR)
				_message_body += "</td><td align=\"right\">" + std::to_string(buf.st_size) + "</td>";
			else
				_message_body += "</td><td align=\"right\"> - </td>";
			_message_body += '\n';
		}
		closedir(dir);	
		_message_body += "</table></hr></body>\n</html>\n";
		//buildBodyDirectory(directoryPath)
	}

	std::cout << "buildBodyDirectory" << std::endl;
	std::cout << "directoryPath: " << directoryPath << std::endl;
}

void HttpResponse::buildBodyFile(std::string requestedFile)
{
	std::cout << "buildBodyFile" << std::endl;
	std::cout << "requestedFile: " << requestedFile << std::endl;
}

void HttpResponse::setBodyHtml(std::string pathHtmlPage)
{
	std::cout << "setBodyHtml" << std::endl;
	std::cout << "pathHtmlPage: " << pathHtmlPage << std::endl;

}

void	 HttpResponse::setMessageBody( Server server )
{
	std::map<std::string, int>::iterator it;
	std::string errorPage = server.getErrorPage(_status_code);
	if (errorPage.empty() && _status_code != 200)
	{
		std::ifstream error("default_error.html"); //taking file as inputstream
		std::string e;
		if (error.is_open())
		{
			std::ostringstream ss;
			ss << error.rdbuf(); // reading data
			e = ss.str();
			replace(e, "[ERRORCODE]", std::to_string(_status_code));
			replace(e, "[REASONPHRASE]", _reason_phrase);
			_message_body = e;
		}
		else
		{
			std::cout << "Error opening file";
		}
	}
	else if (_status_code != 200)
	{
		std::ifstream error(server.getRoot() + errorPage); //taking file as inputstream
		std::string e;
		if (error.is_open())
		{
			std::ostringstream ss;
			ss << error.rdbuf(); // reading data
			e = ss.str();
			_message_body = e;
		}
		else
		{
			std::cout << "Error opening file";
		}
	}
	else if (_message_body.empty())
	{
		std::ifstream f("src/http/index.html"); //taking file as inputstream
		std::string s;
		if (f.is_open())
		{
			std::ostringstream ss;
			ss << f.rdbuf(); // reading data
			s = ss.str();
			_message_body = s;
		}
		else
		{
			std::cout << "Error opening file";
		}
	}
}

std::string HttpResponse::buildResponse( Server server)
{
	std::string str;
	if (!_status_code)
		_status_code = 200;
	if (_reason_phrase.empty())
		_reason_phrase = "OK";
	if (_content_type.empty())
		_content_type = "text/html";
	setMessageBody(server);
	str += "HTTP/1.1 ";
	str += std::to_string(_status_code);
	str += " ";
	str += _reason_phrase;
	str += "\r\n";
	str += "Content-Type: ";
	str += _content_type;
	if (!server.getServerNames()[0].empty())
		str += "\r\nServer: " + server.getServerNames()[0] + "\r\n";
	str += "Content-Length: ";
	str += std::to_string(_message_body.size());
	str += "\r\n\r\n";
	str += _message_body;
	str += "\r\n";
	return (str);
}
