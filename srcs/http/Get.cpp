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

	JSON::Node *matchLocation(JSON::Node *serv, std::string const &path)
	{
		JSON::Object *tmp;
		size_t last_len = 0;
		JSON::Node *last_match = 0;
		size_t i = 0;

		tmp = dynamic_cast<JSON::Object *>(serv->search(1, "location"));
		if (!tmp)
		{
			DEBUG2("no location");
			return 0;
		}
		for (JSON::Node::iterator loc = tmp->begin(); loc != tmp->end(); loc.skip())
		{
			DEBUG2("LOOP");
			if (path.size() == 1)
				i = path.find_first_of(*path.c_str());
			else
				i = path.find(loc->getProperty());
			if (i != std::string::npos && i == 0)
			{
				i = loc->getProperty().size();
				if (path.size() == i)
					return &*loc;
				if (i > last_len)
				{
					last_len = i;
					last_match = &*loc;
				}
			}
		}
		DEBUG2("END");
		return last_match;
	}

	void Get::response(Client & client)
	{
		if (client.sending())
			return contentEncoding(client);
		
		std::string		path;
		JSON::Node *	 location = 0;
		JSON::Node *	 var = 0;

		if (client.req.getMethod()[1].find(".py") != std::string::npos)
			return cgi(client);
		// if (client.server)
		// 	location = matchLocation(client.server, client.req.getMethod()[1]);
		location = client.location;
		if (!location)
			return dirIndex(client, path, location);
		if (!(var = location->search(1, "root")))
			return client.error(404, false);

		path = var->as<std::string const&>();
		if (*--path.end() == '/')
			path.erase(--path.end());
		path += client.req.getMethod()[1];
		if (!(client.fp = getFile(client, path, location)))
			return ;

		client.res.createMethodVec("HTTP/1.1 200 OK");
		size_t index = path.find(".");
		if (index == std::string::npos)
			client.res.setField("content-type", "text/html");
		else
		{
			std::map<std::string, std::string>::const_iterator
					mime_val =	Server::mime.find(path.c_str() + index + 1);
			if (mime_val != Server::mime.end())
				client.res.setField("content-type", mime_val->second);
			else
				client.res.setField("content-type", "text/html");
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
		contentEncoding(client);
	};

	FILE * Get::getFile(Client & client, std::string & path, JSON::Node * location)
	{
		struct stat		stat;
		std::string		path_index;
		FILE*			fp = fopen(path.c_str(), "r");
		JSON::Node *	var = 0;

		if (fp == NULL && errno == ENOENT) {
			client.error(404, false);
			return NULL;
		}
		else if (fp == NULL && errno == EACCES) {
			client.error(403, false);
			return NULL;
		}
		if (lstat(path.c_str(), &stat) == -1) {
			fclose(fp);
			client.error(404, false); // temporary ?
			return NULL;
		}
		if (S_ISDIR(stat.st_mode))
		{
			var = location->search(1, "index");
			if (var)
			{
				path_index = path + var->as<std::string const&>();
				fclose(fp);
				if ((fp = fopen(path_index.c_str(), "r")) == NULL) {
					if (errno == ENOENT) {
						dirIndex(client, path, location);
						return NULL;
					}
					else if (errno == EACCES) {
						client.error(403, false);
						return NULL;
					}
				}
				path = path_index;
			}
			else
			{
				dirIndex(client, path, location);
				return NULL;
			}
		}
		return fp;
	}

	void Get::contentEncoding(Client & client) 
	{
		char				buf[S_BUFFER_SIZE];
		std::string			str;
		int					read_nbr;

		memset(buf, 0, S_BUFFER_SIZE);

		read_nbr = fread(buf, 1, S_BUFFER_SIZE, client.fp);
		if (read_nbr != S_BUFFER_SIZE && client.ok()) {
			client.res.setField("content-length", itos(read_nbr, std::dec));
			client.res.body.assign(buf, read_nbr);
			str = client.res.toString();
			if (send(client.fd, str.c_str(), str.size(), 0) == -1)
				client.error(500, true);
		}
		else
		{
			if (client.ok()) {
				client.res.setField("transfer-encoding", "chunked");
				str = client.res.toString();
				client.setSending();
			}
			if (read_nbr != 0) {
				str += itos(read_nbr, std::hex) + "\r\n";
				client.res.body.assign(buf, read_nbr);
				str += client.res.body + "\r\n";
				if (send(client.fd, str.c_str(), str.size(), 0) == -1) {
					client.error(500, true);
					client.setOk();
				}
				client.res.body.clear();
			}
			else {
				if (send(client.fd, "0\r\n\r\n", 5, 0) == -1)
					client.error(500, true);
				fclose(client.fp);
				client.setOk();
			}
		}
	};

	void Get::dirIndex(Client & client, std::string const& path, JSON::Node * location)
	{
		JSON::Node * autoindex;
		std::string	 str;

		if (!location)
			return client.error(404, false);
		client.res.createMethodVec("HTTP/1.1 200 OK");
		client.res.setField("content-type", "text/html");
		// client.res.setField("date", getDate(time(0)));

		autoindex = location->search(1, "autoindex");
		if (!autoindex || !autoindex->as<bool>())
			return client.error(404, false);

		DIR * dirp = opendir(path.c_str());
		if (!dirp)
			return client.error(403, false);
		client.res.body = "<html>\n<headfile explorer</head>\n<body>\n<hr><pre><a href=\"../\"/>../</a>\n";
		dirent * dp;
		while ((dp = readdir(dirp)) != NULL)
		{
			struct stat stat;
			if (dp->d_name[0] != '.')
			{
				if (lstat(std::string(path.c_str() + std::string(dp->d_name)).c_str(), &stat) != -1
					&& S_ISDIR(stat.st_mode))
				{
					client.res.body += "<a href=\"" + std::string(dp->d_name) \
					+ "/\"/>" + std::string(dp->d_name) + "/</a>\n";
				}
				else
				{
					client.res.body += "<a href=\"" + std::string(dp->d_name) \
					+ "\"/>" + std::string(dp->d_name) + "</a>\n";
				}
			}
		}
		closedir(dirp);
		client.res.body += "</hr></pre>\n</body>\n</html>\n";
		client.res.setField("content-length", itos(client.res.body.length(), std::dec));
		if (client.req.getField("connection")
			&& *client.req.getField("connection") == "close")
			client.res.setField("connection", "close");
		else
			client.res.setField("connection", "keep-alive");
		str = client.res.toString();
		send(client.fd, str.c_str(), str.size(), 0);
		return ;
	};
}
