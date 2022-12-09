#include "GetHandler.hpp"

namespace HTTP {

	GetHandler::GetHandler(std::map<std::string, std::string> const& m): ARequestHandler(), mime(m) {
	}

	GetHandler::~GetHandler(void) {
	}

	GetHandler::GetHandler(GetHandler const& cpy): ARequestHandler(), mime(cpy.mime) {};

	GetHandler&	GetHandler::operator=(GetHandler const& rhs) {
		(void)rhs;
		return *this;
	};

	// static bool replace(std::string& str, const std::string& from, const std::string& to) {
	// 	size_t start_pos = str.find(from);
	// 	if (start_pos == std::string::npos)
	// 		return false;
	// 	str.replace(start_pos, from.length(), to);
	// 	return true;
	// }

	void GetHandler::execute(Message const& req, int client_fd) {
		if (req.getMethod()[1].find(".py") != std::string::npos) //Deal with names containing .cgi elsewhere
		{
			cgiDealer(req, client_fd);
			return;
		}

		DEBUG2("GET");

		std::string	path;
		FILE*		fp;
		Message		res;


		if (req.conf)
		{
			// find full path
			try {
				path = (*req.conf)["root"].as<std::string const&>();
				if (*--path.end() == '/')
					path.erase(--path.end());
				path += req.getMethod()[1];
				DEBUG2("PATH OK");
				fp = getFile(req, path, client_fd);
				if (fp == NULL)
					return;
				DEBUG2("path = " << path);
			}
			catch (std::exception const&) {path.clear();}

			if (path.empty())
				return dirIndex(req, client_fd, path);
		}
		else
			return dirIndex(req, client_fd, path);

		res.createMethodVec("HTTP/1.1 200 OK");
		size_t index = path.find(".");
		//temporary mimes
		//DEBUG2("str = " << str);
		// char const* dot = strrchr(str , '.');
		if (index == std::string::npos)
			res.add("content-type", "text/html"); // default
		else
		{
			//DEBUG2("type = " << *dot);
			std::map<std::string, std::string>::const_iterator
					mime_val =	mime.find(path.c_str() + index + 1);
			if (mime_val != mime.end())
				res.add("content-type", mime_val->second);
			else
				res.add("content-type", "text/html"); // default
		}
		// set all standart headers
		res.add("server", "Webserv/0.3");
		res.add("date", getDate(time(0)));
		if (req.getHeaderVal("connection") == "close")
			res.add("connection", "close");
		else
			res.add("connection", "keep-alive");
		//Last-Modified header creation
		struct stat f_info;
		lstat(path.c_str(), &f_info);
		res.add("last-modified", getDate(f_info.st_mtime));
		contentEncoding(fp, client_fd, res);
	};

	FILE*	GetHandler::getFile(Message const& req, std::string& path, int const& client_fd){
		struct stat		stat;
		std::string		path_index;
		FILE*			fp = fopen(path.c_str(), "r");

		if (fp == NULL && errno == ENOENT) {
			errorPage(req, client_fd, 404);
			return NULL;
		}
		else if (fp == NULL && errno == EACCES) {
			errorPage(req, client_fd, 403);
			return NULL;
		}
		if (lstat(path.c_str(), &stat) == -1) {
			fclose(fp);
			errorPage(req, client_fd, 404);        //temporary
			return NULL;
		}
		if (S_ISDIR(stat.st_mode))
		{
			try
			{
				path_index = path + (*req.conf)["index"].as<std::string const&>();
				DEBUG2("path = " << path_index);

				fclose(fp);
				if ((fp = fopen(path_index.c_str(), "r")) == NULL) {
					if (errno == ENOENT) {
						dirIndex(req, client_fd, path);
						return NULL;
					}
					else if (errno == EACCES) {
						errorPage(req, client_fd, 403);
						return NULL;
					}
				}
				path = path_index;
			}
			catch (std::exception const&) {
				dirIndex(req, client_fd, path);
				return NULL;
			}
		}
		return fp;
	}
	void	GetHandler::contentEncoding(FILE * fp, int client_fd, Message& resp) {
		
		char				buf[READ_BUF_SIZE];
		std::stringstream	ss;
		std::string			str;
		int					read_nbr;
		std::string			new_buf;
		
		memset(buf, 0, READ_BUF_SIZE);
		read_nbr = fread(buf, 1, READ_BUF_SIZE, fp);
		DEBUG2("BYTES READ: " << read_nbr);
		if (read_nbr != READ_BUF_SIZE) {
			ss << read_nbr;
			ss >> str;
			resp.add("content-length", str);
			new_buf.assign(buf, read_nbr);
			resp.setBody(new_buf);
			str = resp.responseString();
			send(client_fd, str.c_str(), str.size(), 0);
		}
		else
		{
			resp.add("transfer-encoding", "chunked");
			str = resp.responseString();
			str += "\r\n";
			send(client_fd, str.c_str(), str.size(), 0);
			str.clear();
			while (read_nbr != 0) {
				ss << std::hex << read_nbr;
				ss >> str;
				ss.clear();
				str += "\r\n";
				new_buf.assign(buf, read_nbr);
				str += new_buf + "\r\n";
				send(client_fd, str.c_str(), str.size(), 0);
				memset(buf, 0, READ_BUF_SIZE);
				new_buf.clear();
				str.clear();
				read_nbr = fread(buf, 1, READ_BUF_SIZE, fp);
			}
			send(client_fd, "0\r\n\r\n", 5, 0);
		}
		fclose(fp);
	};

	void	GetHandler::dirIndex(Message const& req, int fd, std::string const& path) {

		std::string	str;
		Message		res;

		DEBUG2("INDEXING");
		res.createMethodVec("HTTP/1.1 404 Not Found");
		res.add("content-type", "text/html");
		res.add("date", getDate(time(0)));
		try
		{
			if (!req.conf)
				throw std::logic_error("request doesn't match any configuration");
			if ((*req.conf)["autoindex"].as<bool>())
			{
				str = "<html><head>file explorer</head><body><hr><pre>";
				DIR * dirp = opendir(path.c_str());
				if (!dirp)
					throw std::invalid_argument("no such directory");
				dirent * dp;
				while ((dp = readdir(dirp)) != NULL)
				{
					struct stat stat;
					if (dp->d_name[0] != '.')
					{
						if (lstat(std::string(path.c_str() + std::string(dp->d_name)).c_str(), &stat) != -1
							&& S_ISDIR(stat.st_mode))
						{
							str += "<a href=\"" + std::string(dp->d_name) \
							+ "/\"/>" + std::string(dp->d_name) + "/</a>\n";
						}
						else
						{
							str += "<a href=\"" + std::string(dp->d_name) \
							+ "\"/>" + std::string(dp->d_name) + "</a>\n";
						}
					}
				}
				closedir(dirp);
				str += "</hr></pre></body></html>";
				std::ostringstream ss;
				ss << str.length();
				res.add("content-length", ss.str());
				res.setBody(str);
			}
			else
				throw std::invalid_argument("autoindex: false");
		}
		catch(const std::exception& e)
		{
			DEBUG2(e.what());
			// TODO: error_page
			res.add("content-length", "12");
			res.setBody("<h1>404</h1>");
		}
		str = res.responseString();
		send(fd, str.c_str(), str.size(), 0);
		DEBUG2("RESPONSE SENT");
		return ;
	};
}
