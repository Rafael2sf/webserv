#include "HTTPReq.hpp"

namespace ft {

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

		std::cout << "The method is: " << method << std::endl;
	}

	void	HTTPReq::pair_create_add(std::string str) {
		
		std::string		key(wp_trimmer(str.substr(0, str.find(":"))));
		std::string 	value(wp_trimmer(str.substr(str.find(":") + 1)));
		
		headers.insert(std::make_pair(key, value));
	};

	std::string	HTTPReq::wp_trimmer(std::string const& str) {

		if (str.empty())
			return str;
		
		int	start = str.find_first_not_of(" \t\n\r\v\f");
		int	finish = str.find_last_not_of(" \t\n\r\v\f");
		
		return str.substr(start, finish - start + 1);
	};
}