#include "Get.hpp"
#include "Server.hpp"
#include "webserv.hpp"

namespace HTTP
{
	Get::~Get(void)
	{}

	Get::Get( void )
	{}

	Get&	Get::operator=(Get const& rhs)
	{
		(void)rhs;
		return *this;
	};

	void Get::response(Client & client)
	{
		std::string		path;
		JSON::Node *	var = 0;

		if (!client.location)
			return client.error(404, false);
		if (client.location->search(1, "cgi")
			 && getFileExtension(client.req.getMethod()[1]) == "py")
			return cgi(client, "");
		var = client.location->search(1, "root");
		if (!var)
			path = std::string("./html/");
		else if (var->as<std::string const&>().empty())
			return client.error(404, false);
		else
			path = var->as<std::string const&>();
		if (*--path.end() == '/')
			path.erase(--path.end());
		path += client.req.getMethod()[1];
		if (!(client.getFile(path)))
			return ;
		if (client.location->search(1, "cgi")
			&& getFileExtension(path) == "py")
		{
			size_t index = path.find_last_of('/');
			if (index == std::string::npos)
				index = 0;
			return cgi(client, path.substr(index));
		}
		client.res.createMethodVec("HTTP/1.1 200 OK");
		std::map<std::string, std::string>::const_iterator
				mime_val =	Server::mime.find(getFileExtension(path));
		if (mime_val != Server::mime.end())
			client.res.setField("content-type", mime_val->second);
		else
			client.res.setField("content-type", "application/octet-stream");
		if (client._checkAccept(mime_val->second) == -1)
			return client.error(406, false);
		if ((client.req.getField("connection")
				&& *client.req.getField("connection") == "close")
			|| (!client.res.getField("connection")
				&& client.req.getMethod()[2] == "HTTP/1.0"))
			client.res.setField("connection", "close");
		else
			client.res.setField("connection", "keep-alive");
		//Last-Modified header creation
		struct stat f_info;
		lstat(path.c_str(), &f_info);
		client.res.setField("last-modified", getDate(f_info.st_mtime));
		client.contentEncoding();	//Any errors here will set the state for client cleanup after _methodChoice() in server class.
	};
}
