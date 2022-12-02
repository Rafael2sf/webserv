#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include "Mediator.hpp"
#include "Server.hpp"

#define	READ_BUF_SIZE 8000
#define	DATE_BUF_SIZE 40

namespace HTTP {

	Mediator::Mediator(void)
	{
		mime["html"]	=	"text/html";
		mime["htm"]		=	"text/html";
		mime["shtml"]	=	"text/html";
		mime["css"]		=	"text/css";
		mime["xml"]		=	"text/xml";
		mime["gif"]		=	"image/gif";
		mime["jpeg"]	=	"image/jpeg";
		mime["jpg"]		=	"image/jpeg";
		mime["js"]		=	"application/javascript";
		mime["atom"]	=	"application/atom+xml";
		mime["rss"]		=	"application/rss+xml";

		mime["mml"]		=	"text/mathml";
		mime["txt"]		=	"text/plain";
		mime["jad"]		=	"text/vnd.sun.j2me.app-descriptor";
		mime["wml"]		=	"text/vnd.wap.wml";
		mime["htc"]		=	"text/x-component";

		mime["png"]		=	"image/png";
		mime["tif"]		=	"image/tiff";
		mime["tiff"]	=	"image/tiff";
		mime["wbmp"]	=	"image/vnd.wap.wbmp";
		mime["ico"]		=	"image/x-icon";
		mime["jng"]		=	"image/x-jng";
		mime["bmp"]		=	"image/x-ms-bmp";
		mime["svg"]		=	"image/svg+xml";
		mime["svgz"]	=	"image/svg+xml";
		mime["webp"]	=	"image/webp";

		mime["woff"]	=	"application/font-woff";
		mime["jar"]		=	"application/java-archive";
		mime["war"]		=	"application/java-archive";
		mime["ear"]		=	"application/java-archive";
		mime["json"]	=	"application/json";
		mime["hqx"]		=	"application/mac-binhex40";
		mime["doc"]		=	"application/msword";
		mime["pdf"]		=	"application/pdf";
		mime["ps"]		=	"application/postscript";
		mime["eps"]		=	"application/postscript";
		mime["ai"]		=	"application/postscript";
		mime["rtf"]		=	"application/rtf";
		mime["m3u8"]	=	"application/vnd.apple.mpegurl";
		mime["xls"]		=	"application/vnd.ms-excel";
		mime["eot"]		=	"application/vnd.ms-fontobject";
		mime["ppt"]		=	"application/vnd.ms-powerpoint";
		mime["wmlc"]	=	"application/vnd.wap.wmlc";
		mime["kml"]		=	"application/vnd.google-earth.kml+xml";
		mime["kmz"]		=	"application/vnd.google-earth.kmz";
		mime["7z"]		=	"application/x-7z-compressed";
		mime["cco"]		=	"application/x-cocoa";
		mime["jardiff"]	=	"application/x-java-archive-diff";
		mime["jnlp"]	=	"application/x-java-jnlp-file";
		mime["run"]		=	"application/x-makeself";
		mime["pl"]		=	"application/x-perl";
		mime["pm"]		=	"application/x-perl";
		mime["prc"]		=	"application/x-pilot";
		mime["pdb"]		=	"application/x-pilot";
		mime["rar"]		=	"application/x-rar-compressed";
		mime["rpm"]		=	"application/x-redhat-package-manager";
		mime["sea"]		=	"application/x-sea";
		mime["swf"]		=	"application/x-shockwave-flash";
		mime["sit"]		=	"application/x-stuffit";
		mime["tcl"]		=	"application/x-tcl";
		mime["tk"]		=	"application/x-tcl";
		mime["der"]		=	"application/x-x509-ca-cert";
		mime["pem"]		=	"application/x-x509-ca-cert";
		mime["crt"]		=	"application/x-x509-ca-cert";
		mime["xpi"]		=	"application/x-xpinstall";
		mime["xhtml"]	=	"application/xhtml+xml";
		mime["xspf"]	=	"application/xspf+xml";
		mime["zip"]		=	"application/zip";

		mime["bin"]		=	"application/octet-stream";
		mime["exe"]		=	"application/octet-stream";
		mime["dll"]		=	"application/octet-stream";
		mime["deb"]		=	"application/octet-stream";
		mime["dmg"]		=	"application/octet-stream";
		mime["iso"]		=	"application/octet-stream";
		mime["img"]		=	"application/octet-stream";
		mime["msi"]		=	"application/octet-stream";
		mime["msp"]		=	"application/octet-stream";
		mime["msm"]		=	"application/octet-stream";

		mime["docx"]	=	\
		 "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
		mime["xlsx"]	=	\
		 "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
		mime["pptx"]	=	\
		 "application/vnd.openxmlformats-officedocument.presentationml.presentation";

		mime["mid"]		=	"audio/midi";
		mime["midi"]	=	"audio/midi";
		mime["kar"]		=	"audio/midi";
		mime["mp3"]		=	"audio/mpeg";
		mime["ogg"]		=	"audio/ogg";
		mime["m4a"]		=	"audio/x-m4a";
		mime["ra"]		=	"audio/x-realaudio";

		mime["3gpp"]	=	"video/3gpp";
		mime["3gp"]		=	"video/3gpp";
		mime["ts"]		=	"video/mp2t";
		mime["mp4"]		=	"video/mp4";
		mime["mpeg"]	=	"video/mpeg";
		mime["mpg"]		=	"video/mpeg";
		mime["mov"]		=	"video/quicktime";
		mime["webm"]	=	"video/webm";
		mime["flv"]		=	"video/x-flv";
		mime["m4v"]		=	"video/x-m4v";
		mime["mng"]		=	"video/x-mng";
		mime["asx"]		=	"video/x-ms-asf";
		mime["asf"]		=	"video/x-ms-asf";
		mime["wmv"]		=	"video/x-ms-wmv";
		mime["avi"]		=	"video/x-msvideo";
	}

	void	Mediator::methodChoice(Message& req, int client_fd)
	{
		std::vector<std::string>	method(req.getMethod());

		if (method[0] =="POST")
			post(req, client_fd);
		else if (method[0] == "GET") 
			get(req, client_fd);
		else if (method[0] == "DELETE")
			del(req, client_fd);
		else
			send(client_fd, "HTTP/1.1 400 Bad Request\r\n\r\n", 28, 0);
	}

	static bool replace(std::string& str, const std::string& from, const std::string& to) {
		size_t start_pos = str.find(from);
		if (start_pos == std::string::npos)
			return false;
		str.replace(start_pos, from.length(), to);
		return true;
	}

	static void errorPage(Message const& req, Message & res,
		int fd, std::string const& code, std::string const& path)
	{
		std::string	str;

		res.createMethodVec("HTTP/1.1 " + code + " Not Found");
		res.add("content-type", "text/html");
		try
		{
			if (!req.conf)
				throw std::logic_error("request doesn't mach any configuration");
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
			res.setBody("<h1>" + code + "</h1>");
		}
		str = res.responseString();
		send(fd, str.c_str(), str.size(), 0);
		return ;
	}

	void	Mediator::get(Message const& req, int client_fd)
	{
		
		if (req.getMethod()[1].find(".py") != std::string::npos) //Deal with names containing .cgi elsewhere
		{
			cgiDealer(req, client_fd);
			return;
		}

		std::string	path;
		Message		res;

		// set all standart headers
		res.add("server", "Webserv/0.3");
		res.add("date", getDate(time(0)));
		if (req.getHeaderVal("connection") == "close")
			res.add("connection", "close");
		else
			res.add("connection", "keep-alive");

		if (req.conf)
		{
			// find full path
			try {
				path = req.getMethod()[1];
				replace(path,
					req.conf->getProperty(),
					(*req.conf)["root"].as<char const*>());
				DEBUG2("path = " << path);
			}
			catch (std::exception const&) {path.clear();}

			if (path.empty() || !getFile(req, res, path))
				return errorPage(req, res, client_fd, "404", path);
			//Last-Modified header creation
			struct stat f_info;
			stat(path.c_str(), &f_info);
			res.add("last-modified", getDate(f_info.st_mtime));
		}
		else
			return errorPage(req, res, client_fd, "404", path);

		std::fstream	ifs(path.c_str());
		if (!ifs.is_open())
			DEBUG2("I AM CLOSED!!!!");
		contentEncoding(ifs, client_fd, res);
	}

	bool	Mediator::getFile(Message const& req,
		Message& resp, std::string& path)
	{
		std::ifstream	ifs(path.c_str());
		struct stat		stat;
		std::string		path_index;

		if (!ifs.is_open())
			return false;
		if (lstat(path.c_str(), &stat) == -1)
			return false;
		if (S_ISDIR(stat.st_mode))
		{
			try
			{
				path_index = path + (*req.conf)["index"].as<const char*>();
				DEBUG2("path = " << path_index);
				ifs.close();
				ifs.open(path_index.c_str());
				if (!ifs.is_open())
					return false;
				path = path_index;
			}
			catch (std::exception const&) {return false;}
		}

		resp.createMethodVec("HTTP/1.1 200 OK");
		size_t index = path.find(".");
		//temporary mimes
		//DEBUG2("str = " << str);
		// char const* dot = strrchr(str , '.');
		if (index == std::string::npos)
			resp.add("content-type", "text/html"); // default
		else
		{
			//DEBUG2("type = " << *dot);
			std::map<std::string, std::string>::const_iterator
					mime_val =	mime.find(path.c_str() + index + 1);
			if (mime_val != mime.end())
				resp.add("content-type", mime_val->second);
			else
				resp.add("content-type", "text/html"); // default
		}
		return true;
	};

	std::string	Mediator::getDate(time_t const& tm_info) {
		
		tm*		current_time;
		char	buffer[DATE_BUF_SIZE];
		std::string	ret;

		memset(buffer, 0, DATE_BUF_SIZE);
		current_time = localtime(&tm_info);
		strftime(buffer, DATE_BUF_SIZE, "%a, %d %b %Y %X %Z",current_time);
		ret = buffer;
		return ret;
	};

	void		Mediator::contentEncoding(std::fstream & ifs, int client_fd, Message& resp) {
		
		char				buf[READ_BUF_SIZE];
		std::stringstream	ss;
		std::string			str;
		int					read_nbr;
		std::string			new_buf;
		
		memset(buf, 0, READ_BUF_SIZE);
		ifs.read(buf, READ_BUF_SIZE);
		read_nbr = ifs.gcount();
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
				ifs.read(buf, READ_BUF_SIZE);
				read_nbr = ifs.gcount();
			}
			send(client_fd, "0\r\n\r\n", 5, 0);
		}
	};

	void	Mediator::cgiDealer(Message const& req, int client_fd) {

		int	pid, exit_stat;

		pid = fork();
		if (pid == -1)
			DEBUG2("fork failed");
		if (pid == 0) {
			
			CGI	test(req);
			if (dup2(client_fd, STDOUT_FILENO) == -1)
			{
				write(2, "error: fatal\n", 13);
				exit(EXIT_FAILURE);
			}
			execve("/usr/bin/python3", test.getArgs(), test.getEnv());
			DEBUG2("EXECVE FAILED!!");
		}
		else {
			waitpid(pid, &exit_stat, 0);
		}
	};
	
	void	Mediator::post(Message & req, int client_fd) {
		DEBUG2("This is a POST request");
		
		int	exit_stat;
		int p[2];
		pipe(p);
		int	pid = fork();
		if (pid == -1)
			DEBUG2("fork failed");
		if (pid == 0) {
			
			close(p[1]);
			CGI	test(req);
			if (dup2(p[0], STDIN_FILENO) == -1)
			{
				write(2, "error: fatal\n", 13);
				exit(EXIT_FAILURE);
			}
			close (p[0]);
			if (dup2(client_fd, STDOUT_FILENO) == -1)
			{
				write(2, "error: fatal\n", 13);
				exit(EXIT_FAILURE);
			}
			execve("/usr/bin/python3", test.getArgs(), test.getEnv());
			exit (1);
		}
		else {
			close(p[0]);
			std::string playing = req.getBody();
			size_t wsize = 0;
			size_t b;
			for (b = 0; b + 65000 < playing.size(); b += 65000)
			{
				write(p[1], playing.c_str() + b, 65000);
			}
			wsize = playing.size() - b;
			if (wsize > 0)
				write(p[1], playing.c_str() + b, wsize);
			close(p[1]);
			waitpid(pid, &exit_stat, 0);
		}
	}

	void	Mediator::del(Message const& req, int client_fd) {
		DEBUG2("This is a DELETE request");

		int	exit_stat;
		int p[2];
		pipe(p);
		int	pid = fork();
		if (pid == -1)
			DEBUG2("fork failed");
		if (pid == 0) {
			
			close(p[1]);
			CGI	test(req);
			if (dup2(p[0], STDIN_FILENO) == -1)
			{
				write(2, "error: fatal\n", 13);
				exit(EXIT_FAILURE);
			}
			close (p[0]);
			if (dup2(client_fd, STDOUT_FILENO) == -1)
			{
				write(2, "error: fatal\n", 13);
				exit(EXIT_FAILURE);
			}
			execve("/usr/bin/python3", test.getArgs(), test.getEnv());
			exit (1);
		}
		else {
			close(p[0]);
			std::string playing = req.getBody();
			size_t wsize = 0;
			size_t b;
			for (b = 0; b + 65000 < playing.size(); b += 65000)
			{
				write(p[1], playing.c_str() + b, 65000);
			}
			wsize = playing.size() - b;
			if (wsize > 0)
				write(p[1], playing.c_str() + b, wsize);
			close(p[1]);
			waitpid(pid, &exit_stat, 0);
		}
	}
}
