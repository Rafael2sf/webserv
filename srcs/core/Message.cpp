#include "Message.hpp"

namespace HTTP {

	Message::Message(void) {

	}

	int	Message::init(std::string & req) {
		
		size_t		start = req.find("\n");
	
		if (req.empty())
			return -1;
		createMethodVec(req.substr(0, start++));
		req.erase(0, start);
		start = 0;
		for (size_t i = 0; req[i] != *(req.end()) ; i++) {
			if (req[i] == '\n') {
				std::string	new_header(req.substr(start, i - start - 1));
				strToHeaderPair(new_header);
				start = i + 1;
				if (req[i + 1] == '\r')
					break;
			}
		}
		if (req[start] == '\r' && req[start + 2] != *(req.end()))
			_body = req.assign(&req[start + 2], ftStoi(getHeaderVal("content-length")));
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

	void	Message::printMap(void) const {
		for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); it++) {
			std::cout << "\033[31m" << "header key: " << "\033[0m" << it->first
					<< "\033[32m" << "; header value: " << "\033[0m" << it->second << ';' << std::endl;
		}
		DEBUG2("<<END OF HEADERS>>" << std::endl);
		//std::cout << "The method is: " << method[0] << ", " << method[1] << ", " << method[2] << std::endl;
	}

	void	Message::createMethodVec(std::string const& str) {

		size_t	i = 0, start, end;

		method.resize(4);
		for (end = 0; end < str.size();) {
			start = str.find_first_not_of(' ', end);
			end = str.find_first_of(" \0", start); // Possible problem with '\0'????
			method[i++] = str.substr(start, end - start);
		}
		owsTrimmer(method[2]);						// chars like \n \r could end up staying in the string of the last element
		start = method[1].find_first_of('?');
		if (start != std::string::npos)
		{
			method[3] = method[1].substr(start + 1);
			method[1].erase(method[1].begin() + start, method[1].end());
		}
	}

	void	Message::strToHeaderPair(std::string str) {
		
		std::string		key(str.substr(0, str.find(":")));
		std::string 	value(str.substr(str.find(":") + 1));
		
		owsTrimmer(key);
		owsTrimmer(value);
		std::transform(key.begin(), key.end(), key.begin(), ::tolower);
		headers.insert(std::make_pair(key, value));
	};

	void	Message::owsTrimmer(std::string& str) {

		if (str.empty())
			return ;
		size_t	start = str.find_first_not_of(" \t\n\r\v\f");
		size_t	finish = str.find_last_not_of(" \t\n\r\v\f");
		if (start == std::string::npos)
		{
			str = "";
			return ;
		}
		str = str.substr(start, finish - start + 1);
	};

	std::vector<std::string> const&	Message::getMethod(void) const {
		
		return this->method;
	}

	std::string				Message::getHeaderVal(std::string const& key) const {
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

	void					Message::addToVal(std::string const& key, std::string const& value_to_add) {

		std::map<std::string, std::string>::iterator it = headers.find(key);
		if (it == headers.end())
			DEBUG2("INVALID KEY VALUE IN addToVal()!");
		else
			it->second += value_to_add;

	};


	std::string				Message::responseString(void) {
		
		std::string	final_str;

		for (std::vector<std::string>::iterator it = method.begin(); it != method.end(); it++) {
			final_str += *it + ' ';
		}
		final_str.replace(final_str.size() - 1, 1, "\r\n");
		
		for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); it++)
		{
			final_str += it->first + ": " + it->second + "\r\n";
		}
		final_str += "\r\n" + _body;
		return final_str;
	};
	
	std::string const&			Message::getBody(void) const {
		return this->_body;
	};

	void						Message::setBody(std::string bod) {
		_body = bod;
	};

	int	ftStoi(std::string const& str) {
		
		std::stringstream	ss;
		int					ret;
		
		ss << str;
		ss >> ret;
		return ret;
	}

	std::string	ftItos(int const& n) {
		
		std::stringstream	ss;
		std::string			ret;

		ss << n;
		ss >> ret;
		return ret;
	}

}
