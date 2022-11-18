 #include "Mediator.hpp"
#include "HTTPServer.hpp"
#include <sys/stat.h>
#define	READ_BUF_SIZE 8000
#define	DATE_BUF_SIZE 40

namespace ft {

	Mediator::Mediator(void) {

	}

	void	Mediator::method_choice(HTTPReq const& req, int client_fd) {
		std::vector<std::string>	method(req.get_method());

		if (method.empty())
			send(client_fd, "HTTP/1.1 400 Bad Request\r\n\r\n", 29, 0);
		else if (method[0] == "GET") 
			get(req, client_fd);
		else if (method[0] =="POST")
			post(req, client_fd);
		else if (method[0] == "DELETE")
			del(req, client_fd);
		else
			send(client_fd, "HTTP/1.1 400 Bad Request\r\n\r\n", 29, 0);
	}

	void	Mediator::get(HTTPReq const& req, int client_fd) {

		std::string	str;
		std::string	path("/nfs/homes/daalmeid/Desktop/new_webserv");
		std::vector<std::string> vec = req.get_method();
		HTTPReq	response;

		response.add("Server", "Webserv/0.2");
		response.add("Date", get_date(time(0)));
		if (vec[1] == "/") {
			response.create_vec_method("HTTP/1.1 200 OK");
			response.add("Content-Type", "text/html");
			path += "/html/index.html";
		}
		else if (vec[1] == "/favicon.ico") {
			response.create_vec_method("HTTP/1.1 200 OK");
			response.add("Content-Type", "image/x-icon");
			path += "/favicon/favicon.ico";	
		}
		else
			get_file(req, response, path);
		
		//Last-Modified header creation
		struct stat f_info;
		stat(path.c_str(),&f_info);
		response.add("Last-Modified", get_date(f_info.st_mtime));
		//Ends here
		
		if (req.get_head_val("Connection") == "close")
			response.add("Connection", "close");
		else
			response.add("Connection", "keep-alive");
		std::fstream	ifs(path.c_str());
		content_encoding(ifs, client_fd, response);
	}

	void	Mediator::get_file(HTTPReq const& req, HTTPReq& resp, std::string& path) {

		std::string	req_path(req.get_method()[1]);

		std::fstream	ifs((path + req_path).c_str());
		if (!ifs.is_open()) {
			resp.create_vec_method("HTTP/1.1 404 Not Found");
			resp.add("Content-Type", "text/html");
			path += "/html/404.html";
			return;
		}
		path += req_path;
		resp.create_vec_method("HTTP/1.1 200 OK");
		if (req_path.find("/images/") == 0)
			resp.add("Content-Type", "image/jpeg");
		else if (req_path.find("/html/") == 0)
			resp.add("Content-Type", "text/html");
		return;
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
			//DEBUG2(str);
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

	void	Mediator::post(HTTPReq const& req, int client_fd) {
		(void)req;
		(void)client_fd;
		DEBUG2("This is a POST request");
	}

	void	Mediator::del(HTTPReq const& req, int client_fd) {
		(void)req;
		(void)client_fd;
		DEBUG2("This is a DELETE request");
	}

}
