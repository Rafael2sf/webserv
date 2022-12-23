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
		JSON::Node *	 var = 0;

		if (!client.location)
			return client.error(404, false);
		if (client.req.getMethod()[1].find(".py") != std::string::npos
				&& client.location->search(1, "cgi"))
			return cgi(client);
		if (!(var = client.location->search(1, "root")))
			return client.error(404, false);
		path = var->as<std::string const&>();
		if (*--path.end() == '/')
			path.erase(--path.end());
		path += client.req.getMethod()[1];
		if (!(client.getFile(path)))
			return ;
		client.res.createMethodVec("HTTP/1.1 200 OK");
		size_t index = path.find_last_of(".");
		if (index == std::string::npos)
			client.res.setField("content-type", "application/octet-stream");
		else
		{
			std::map<std::string, std::string>::const_iterator
					mime_val =	Server::mime.find(path.c_str() + index + 1);
			if (mime_val != Server::mime.end())
				client.res.setField("content-type", mime_val->second);
			else
				client.res.setField("content-type", "application/octet-stream");
		}
		if (client.req.getField("connection")
			&& *client.req.getField("connection") == "close")
			client.res.setField("connection", "close");
		else
			client.res.setField("connection", "keep-alive");
		//Last-Modified header creation
		struct stat f_info;
		lstat(path.c_str(), &f_info);
		client.res.setField("last-modified", getDate(f_info.st_mtime));
		client.contentEncoding();
	};
}
