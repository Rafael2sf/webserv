#include "Mediator.hpp"

namespace ft {

	Mediator::Mediator(void) {

	}

	std::string	Mediator::method_choice(HTTPReq const& req) {
		std::vector<std::string>	method(req.get_method());

		if (method.empty())
			return "Empty request";
		if (method[0] == "GET") 
			return get(req);
		else if (method[0] =="POST")
			return post(req);
		else if (method[0] == "DELETE")
			return del(req);
		else
			return "Unrecognized method";
	}

	std::string	Mediator::get(HTTPReq const& req) {

		std::string	str;
		std::string	path("/nfs/homes/daalmeid/Desktop/new_webserv");
		char		buf[30000];
		std::vector<std::string> vec = req.get_method();

		memset(buf, 0, 30000);
		if (vec[1] == "/") {
			str = "HTTP/1.1 200 OK\nContent-Type: text/html\n";
			path += "/html/index.html";
		}
		else if (vec[1] == "/favicon.ico") {
			str = "HTTP/1.1 200 OK\nContent-Type: image/x-icon\n";
			path += "/favicon/favicon.ico";	
		}
		else {
			str = get_file(req, path);
		}
		std::fstream	ifs(path.c_str());
		ifs.read(buf, 30000);
		std::stringstream	ss;
		ss << ifs.gcount();
		std::string	size;
		ss >> size; 
		std::string	new_buf;
		new_buf.assign(buf, ifs.gcount());
		size = "Content-Length: " + size + "\n\n";
		str = str + size + new_buf;
		//std::cout << new_buf << " TADAAAA" << std::endl;
		return str;
	}

	std::string	Mediator::get_file(HTTPReq const& req, std::string& path) {

		std::string	str("HTTP/1.1");
		std::string	req_path(req.get_method()[1]);

		std::fstream	ifs((path + req_path).c_str());
		if (!ifs.is_open()) {
			str += " 404 Not Found\nContent-Type: text/html\n";
			path += "/html/404.html";
			return str;
		}
		path += req_path;
		if (req_path.find("/images/") == 0)
			str += " 200 OK\nContent-Type: image/jpeg\n";
		else if (req_path.find("/html/") == 0)
			str += " 200 OK\nContent-Type: text/html\n";
		return str;
		

	};

	std::string	Mediator::post(HTTPReq const& req) {
		(void)req;
		return "This is a POST request";
	}

	std::string	Mediator::del(HTTPReq const& req) {
		(void)req;
		return "This is a DELETE request";
	}
}