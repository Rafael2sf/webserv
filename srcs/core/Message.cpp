#include "Message.hpp"

namespace HTTP {

	Message::Message(void) {

	}

	int	Message::init(std::string & req) {
		
		size_t		start = req.find("\n");
		
		if (req.empty())
			return -1;
		create_vec_method(req.substr(0, start++));
		req.erase(0, start);
		start = 0;
		for (size_t i = 0; req[i] != *(req.end()) ; i++) {
			if (req[i] == '\n') {
				std::string	new_header(req.substr(start, i - start - 1));
				pair_create_add(new_header);
				start = i + 1;
				if (req[i + 1] == '\r')
					break;
			}
		}
		if (req[start] == '\r' && req[start + 2] != *(req.end()))
			_body = req.assign(&req[start + 2], ftStoi(get_head_val("content-length")));
		else
			_body = "";
		return 0;
	}

	Message::Message(Message const& cpy) {
	(void)cpy;
	}

	Message::~Message(void) {

	}

	Message& Message::operator=(Message const& rhs) {
		(void)rhs;
		return *this;
	}

	void	Message::print_map(void) const {
		for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); it++) {
			std::cout << "\033[31m" << "header key: " << "\033[0m" << it->first
					<< "\033[32m" << "; header value: " << "\033[0m" << it->second << ';' << std::endl;
		}
		DEBUG2("<<END OF HEADERS>>" << std::endl);
		//std::cout << "The method is: " << method[0] << ", " << method[1] << ", " << method[2] << std::endl;
	}

	void	Message::create_vec_method(std::string const& str) {

		size_t	i = 0, start, end;

		method.resize(4);
		for (end = 0; end < str.size();) {
			start = str.find_first_not_of(' ', end);
			end = str.find_first_of(" \0", start); // Possible problem with '\0'????
			method[i++] = str.substr(start, end - start);
		}
		start = method[1].find_first_of('?');
		if (start != std::string::npos)
		{
			method[3] = method[1].substr(start + 1);
			method[1].erase(method[1].begin() + start, method[1].end());
		}
	}

	void	Message::pair_create_add(std::string str) {
		
		std::string		key(wp_trimmer(str.substr(0, str.find(":"))));
		std::string 	value(wp_trimmer(str.substr(str.find(":") + 1)));
		
		std::transform(key.begin(), key.end(), key.begin(), ::tolower);
		headers.insert(std::make_pair(key, value));
	};

	std::string	Message::wp_trimmer(std::string const& str) {

		if (str.empty())
			return str;
		
		size_t	start = str.find_first_not_of(" \t\n\r\v\f");
		size_t	finish = str.find_last_not_of(" \t\n\r\v\f");
		if (start == std::string::npos)
			return ("");
		return str.substr(start, finish - start + 1);
	};

	std::vector<std::string>	Message::get_method(void) const {
		
		return this->method;
	}

	std::string				Message::get_head_val(std::string const& key) const {
		std::map<std::string, std::string>::const_iterator it = headers.find(key);

		if (it != headers.end())
			return it->second;
		return "";
	};

	void					Message::add(std::string key, std::string value) {
		if (key.empty()) {
			DEBUG2("NEED A KEY VALUE, GENIUS!");
			return ;
		}
		headers.insert(std::make_pair(key, value));
	};

	void					Message::addToVal(std::string key, std::string value_to_add) {
		if (key.empty()) {
			DEBUG2("NEED A KEY VALUE, GENIUS!");
			return ;
		}
		headers[key] += value_to_add;
	};


	std::string				Message::response_string(void) {
		
		std::string	final_str;

		for (std::vector<std::string>::iterator it = method.begin(); it != method.end(); it++)
			final_str += *it + ' ';
		final_str.replace(final_str.size() - 1, 1, "\r\n");
		
		for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); it++)
		{
			final_str += it->first + ": " + it->second + "\r\n";
		}
		final_str += "\r\n" + _body;
		return final_str;
	};

	std::string 				Message::getBody(void) const {
		return this->_body;
	};

	void						Message::setBody(std::string bod) {
		_body = bod;
	};

	int					ftStoi(std::string str) {
		std::stringstream	ss;
		int					ret;
		
		ss << str;
		ss >> ret;
		return ret;
	}
}
