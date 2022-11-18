#include "HTTPReq.hpp"
#include "webserv.hpp"


namespace ft {

	HTTPReq::HTTPReq(void) {

	}

	HTTPReq::HTTPReq(char* request) {
		std::string	str(request);
		size_t		str_start = str.find("\n");
		
		if (str.empty())
			return ;
		create_vec_method(str.substr(0, str_start++));
		str = &(request[str_start]);
		str_start = 0;
		
		for (size_t i = 0; str[i] != '\0' ; i++) {
			if (str[i] == '\n') {
				std::string	new_header(str.substr(str_start, i - str_start - 1));
				pair_create_add(new_header);
				str_start = i + 1;
				if (str[i + 1] == '\r')
					break;
			}
		}
		if (str[str_start] == '\r' && str[str_start + 2] != '\0')
			headers.insert(std::make_pair("body", wp_trimmer(str.substr(str_start + 2))));

	}
	
	HTTPReq::HTTPReq(HTTPReq const& cpy) {
	(void)cpy;
	}

	HTTPReq::~HTTPReq(void) {

	}

	HTTPReq& HTTPReq::operator=(HTTPReq const& rhs) {
		(void)rhs;
		return *this;
	}

	void	HTTPReq::print_map(void) const {
		for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); it++) {
			std::cout << "\033[31m" << "header key: " << "\033[0m" << it->first
					<< "\033[32m" << "; header value: " << "\033[0m" << it->second << ';' << std::endl;
		}
		DEBUG2("<<END OF HEADERS>>" << std::endl);
		//std::cout << "The method is: " << method[0] << ", " << method[1] << ", " << method[2] << std::endl;
	}

	void	HTTPReq::create_vec_method(std::string const& str) {

		int	length;
		int	start;

		for (unsigned int i = 0; i < str.size();) {
			start = str.find_first_not_of(' ', i);
			i = start;
			i = str.find_first_of(" \0", start);
			length = i - start;
			method.push_back(str.substr(start, length));
		}
	}

	void	HTTPReq::pair_create_add(std::string str) {
		
		std::string		key(wp_trimmer(str.substr(0, str.find(":"))));
		std::string 	value(wp_trimmer(str.substr(str.find(":") + 1)));
		
		headers.insert(std::make_pair(key, value));
	};

	std::string	HTTPReq::wp_trimmer(std::string const& str) {

		if (str.empty())
			return str;
		
		size_t	start = str.find_first_not_of(" \t\n\r\v\f");
		size_t	finish = str.find_last_not_of(" \t\n\r\v\f");
		if (start == std::string::npos)
			return ("");
		return str.substr(start, finish - start + 1);
	};


	std::vector<std::string>	HTTPReq::get_method(void) const {
		
		return this->method;
	}

	std::string				HTTPReq::get_head_val(std::string const& key) const {
		std::map<std::string, std::string>::const_iterator it = headers.find(key);

		if (it != headers.end())
			return it->second;
		return "";
	};

	void					HTTPReq::add(std::string key, std::string value) {
		if (key.empty()) {
			DEBUG2("NEED A KEY VALUE, GENIUS!");
			return ;
		}
		headers.insert(std::make_pair(key, value));
	};

	std::string				HTTPReq::response_string(void) {
		
		std::string	final_str;

		for (std::vector<std::string>::iterator it = method.begin(); it != method.end(); it++)
			final_str += *it + ' ';
		final_str.replace(final_str.size() - 1, 1, "\r\n");
		
		for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); it++)
		{
			if (it->first == "body")
				final_str += "\r\n" + it->second + "\r\n";
			else
				final_str += it->first + ": " + it->second + "\r\n";
		}
		final_str += "\r\n";
		return final_str;
	};

}