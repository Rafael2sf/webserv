#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include "Mediator.hpp"
#include "HTTPServer.hpp"

#define	READ_BUF_SIZE 8000
#define	DATE_BUF_SIZE 40

namespace ft {

	Mediator::Mediator(void) {
	}

	void	Mediator::method_choice(HTTPReq& req, int client_fd)
	{
		std::vector<std::string>	method(req.get_method());

		if (method[0] =="POST")
			post(req, client_fd);
		else if (method[1].find(".py") != std::string::npos) //Deal with names containing .cgi elsewhere
			cgi_dealer(req, client_fd);
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

	static void errorPage(HTTPReq const& req, HTTPReq & res,
		int fd, std::string const& code, std::string const& path)
	{
		std::string	str;

		res.create_vec_method("HTTP/1.1 " + code + " Not Found");
		res.add("Content-Type", "text/html");
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
				res.add("Content-length", ss.str());
				res.add("body", str);
			}
			else
				throw std::invalid_argument("autoindex: false");
		}
		catch(const std::exception& e)
		{
			DEBUG2(e.what());
			// TODO: error_page
			res.add("Content-length", "16");
			res.add("body", "<h1>" + code + "</h1>");
		}
		str = res.response_string();
		send(fd, str.c_str(), str.size(), 0);
		return ;
	}

	void	Mediator::get(HTTPReq const& req, int client_fd)
	{
		std::string	path;
		HTTPReq		res;

		// set all standart headers
		res.add("Server", "Webserv/0.2");
		res.add("Date", get_date(time(0)));
		if (req.get_head_val("Connection") == "close")
			res.add("Connection", "close");
		else
			res.add("Connection", "keep-alive");

		if (req.conf)
		{
			// find full path
			try {
				path = req.get_method()[1];
				replace(path,
					req.conf->getProperty(),
					(*req.conf)["root"].as<char const*>());
				DEBUG2("path = " << path);
			}
			catch (std::exception const&) {path.clear();}

			if (path.empty() || !get_file(req, res, path))
				return errorPage(req, res, client_fd, "404", path);
			//Last-Modified header creation
			struct stat f_info;
			stat(path.c_str(), &f_info);
			res.add("Last-Modified", get_date(f_info.st_mtime));
		}
		else
			return errorPage(req, res, client_fd, "404", path);

		std::fstream	ifs(path.c_str());
		content_encoding(ifs, client_fd, res);
	}

	bool	Mediator::get_file(HTTPReq const& req,
		HTTPReq& resp, std::string& path)
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

		// temporary mimes
		resp.create_vec_method("HTTP/1.1 200 OK");
		if (path.find(".jpeg") != std::string::npos 
			|| path.find(".jpg") != std::string::npos)
			resp.add("Content-Type", "image/jpeg");
		else if (path.find(".html") != std::string::npos)
			resp.add("Content-Type", "text/html");
		else if (path.find(".ico") != std::string::npos)
			resp.add("Content-Type", "image/x-icon");
		return true;
	};

	std::string	Mediator::get_date(time_t now) {
		
		tm*		current_time;
		char	buffer[DATE_BUF_SIZE];
		std::string	ret;

		memset(buffer, 0, DATE_BUF_SIZE);
		current_time = localtime(&now);
		strftime(buffer, DATE_BUF_SIZE, "%a, %d %b %Y %X %Z",current_time);
		ret = buffer;
		return ret;
	};

	void		Mediator::content_encoding(std::fstream & ifs, int client_fd, HTTPReq& resp) {
		
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
			resp.add("Content-Length", str);
			new_buf.assign(buf, read_nbr);
			resp.add("body", new_buf);
			str = resp.response_string();
			send(client_fd, str.c_str(), str.size(), 0);
		}
		else
		{
			resp.add("Transfer-Encoding", "chunked");
			str = resp.response_string();
			send(client_fd, str.c_str(), str.size(), 0);
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

	void	Mediator::cgi_dealer(HTTPReq const& req, int client_fd) {

		int	pid, exit_stat;
		std::string	path("/nfs/homes/daalmeid/Desktop/webserv");
		path += req.get_method()[1];
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
	
	void	Mediator::post(HTTPReq & req, int client_fd) {
		DEBUG2("This is a POST request");
		int	read_nbr = 1;
		char buf[30000];
		std::string	new_buf;
		
		read_nbr = recv(client_fd, buf, 30000, 0);
		while (read_nbr > 0) {
			new_buf.assign(buf, read_nbr);
			req.addToVal("body", new_buf);
			memset(buf, 0, 30000);
			new_buf.clear();
			read_nbr = recv(client_fd, buf, 30000, 0);
			//DEBUG2(read_nbr);
		}
		
		std::string playing = req.get_head_val("body");

		int	exit_stat;
		int p[2];
		pipe(p);
		playing += "\r\n";
		int	pid = fork();
		if (pid == -1)
			DEBUG2("fork failed");
		if (pid == 0) {
			
			close(p[1]);
			CGI	test(req);
			//write(p[1], playing.c_str(), playing.size());
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
			size_t wsize = 0;
			for (size_t b = 0; b < playing.size(); b += 65536)
			{
				wsize = 65536;
				if (b > playing.size())
					wsize = b - playing.size();
				write(p[1], playing.c_str() + b, wsize);
			}
			close(p[1]);
			waitpid(pid, &exit_stat, 0);
		}
	}

	void	Mediator::del(HTTPReq const& req, int client_fd) {
		(void)req;
		(void)client_fd;
		DEBUG2("This is a DELETE request");
	}
}
