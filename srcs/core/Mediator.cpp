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
			DEBUG2("Empty request");
		else if (method[0] == "GET") 
			get(req, client_fd);
		else if (method[0] =="POST")
			post(req, client_fd);
		else if (method[0] == "DELETE")
			del(req, client_fd);
		else
			DEBUG2("Wrong method");
	}

	void	Mediator::get(HTTPReq const& req, int client_fd) {

		std::string	str;
		std::string	path("/nfs/homes/daalmeid/Desktop/new_webserv");
		std::vector<std::string> vec = req.get_method();
		//HTTPReq	response;

		if (vec[1] == "/") {
			str = "HTTP/1.1 200 OK\r\nServer:Webserv/0.2\r\nDate: " 
			+ get_date(time(0)) + "Content-Type: text/html\r\n";
			path += "/html/index.html";
		}
		else if (vec[1] == "/favicon.ico") {
			str = "HTTP/1.1 200 OK\r\nServer:Webserv/0.2\r\nDate: "
			+ get_date(time(0)) + "Content-Type: image/x-icon\r\n";
			path += "/favicon/favicon.ico";	
		}
		else
			str = get_file(req, path);
		
		//Last-Modified header creation
		struct stat f_info;
		stat(path.c_str(),&f_info);
		str += "Last-Modified: " + get_date(f_info.st_mtime);
		//Ends here
		
		if (req.get_head_val("Connection") == "close")
			str += "Connection: close\r\n";
		else
			str += "Connection: keep-alive\r\n";
			
		std::fstream	ifs(path.c_str());
		content_encoding(ifs, str, client_fd);
	}

	std::string	Mediator::get_file(HTTPReq const& req, std::string& path) {

		std::string	str("HTTP/1.1");
		std::string	req_path(req.get_method()[1]);

		std::fstream	ifs((path + req_path).c_str());
		if (!ifs.is_open()) {
			str += " 404 Not Found\r\nServer:Webserv/0.2\r\nDate: "
			+ get_date(time(0)) + "Content-Type: text/html\r\n";
			path += "/html/404.html";
			return str;
		}
		path += req_path;
		if (req_path.find("/images/") == 0)
			str += " 200 OK\r\nServer:Webserv/0.2\r\nDate: "
			+ get_date(time(0)) + "Content-Type: image/jpeg\r\n";
		else if (req_path.find("/html/") == 0)
			str += " 200 OK\r\nServer: Webserv/0.2\r\nDate: "
			+ get_date(time(0)) + "Content-Type: text/html\r\n";
		return str;
		

	};

	std::string	Mediator::get_date(time_t now) {
		
		tm*		current_time;
		char	buffer[DATE_BUF_SIZE];
		std::string	ret;

		memset(buffer, 0, DATE_BUF_SIZE);
		current_time = localtime(&now);
		strftime(buffer, DATE_BUF_SIZE, "%a, %d %b %Y %X %Z\r\n",current_time);
		ret = buffer;
		return ret;
	};

	void		Mediator::content_encoding(std::fstream & ifs, std::string& str, int client_fd) {
		
		char				buf[READ_BUF_SIZE];
		std::stringstream	ss;
		std::string			size;
		int					read_nbr;
		std::string			new_buf;
		
		memset(buf, 0, READ_BUF_SIZE);
		ifs.read(buf, READ_BUF_SIZE);
		read_nbr = ifs.gcount();
		if (read_nbr != READ_BUF_SIZE) {
			ss << read_nbr;
			ss >> size; 
			size = "Content-Length: " + size + "\r\n\r\n";
			new_buf.assign(buf, read_nbr);
			str = str + size + new_buf;
			send(client_fd, str.c_str(), str.size(), 0);
			//DEBUG2(str);
		}
		else
		{
			str += "Transfer-Encoding: chunked\r\n\r\n";
			send(client_fd, str.c_str(), str.size(), 0);

			while (read_nbr != 0) {
				ss << std::hex << read_nbr;
				ss >> size;
				ss.clear();
				size += "\r\n";
				new_buf.assign(buf, read_nbr);
				size += new_buf + "\r\n";
				send(client_fd, size.c_str(), size.size(), 0);
				memset(buf, 0, READ_BUF_SIZE);
				new_buf.clear();
				size.clear();
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
