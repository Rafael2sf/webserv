#include "Message.hpp"
#include "Client.hpp"

namespace HTTP
{
	Message::~Message(void)
	{}

	Message::Message(void)
	: content_length(0), header_bytes(0)
	{}

	Message::Message(Message const& cpy)
	{
		(void)cpy;
	}

	Message& Message::operator=(Message const& rhs)
	{
		(void)rhs;
		return *this;
	}

	void Message::printMap(void) const
	{
		for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); it++) {
			std::cout << "\033[31m" << "header key: " << "\033[0m" << it->first
					<< "\033[32m" << "; header value: " << "\033[0m" << it->second << ';' << std::endl;
		}
		DEBUG2("<<END OF HEADERS>>" << std::endl);
		//std::cout << "The method is: " << method[0] << ", " << method[1] << ", " << method[2] << std::endl;
	}

	void Message::createMethodVec(std::string const& str)
	{
		std::string s;
		std::stringstream ss(str);

		while (ss >> s)
			method.push_back(s);
	}

	std::vector<std::string> const&	Message::getMethod(void) const
	{
		return this->method;
	}

	// std::string & Message::getMethodAt( size_t i )
	// {
	// 	if (i >= method.size())
	// 		DEBUG2(" BAD INDEX ");
	// 	return this->method[i];
	// }

	std::string * Message::getField(std::string const& key)
	{
		std::map<std::string, std::string>::iterator it =
			headers.find(key);

		if (it != headers.end())
			return &it->second;
		return 0;
	};

	std::string const* Message::getField(std::string const& key) const
	{
		std::map<std::string, std::string>::const_iterator it =
			 headers.find(key);

		if (it != headers.end())
			return &it->second;
		return 0;
	};

	void Message::setField(std::string const& key, std::string const& value)
	{
		if (key.empty()) {
			DEBUG2("NEED A KEY VALUE, GENIUS!");
			return ;
		}
		headers.insert(std::make_pair(key, value));
	};

	std::string Message::toString(void)
	{
		std::string	final_str;

		setField("server", "Webserv/0.4");
		setField("date", getDate(time(0)));
		for (std::vector<std::string>::iterator it =
			method.begin(); it != method.end(); it++)
		{
			if (!it->empty())
				final_str += *it + ' ';
		}
		final_str.replace(final_str.size() - 1, 1, "\r\n");
		for (std::map<std::string, std::string>::iterator it =
			headers.begin(); it != headers.end(); it++)
		{
			final_str += it->first + ": " + it->second + "\r\n";
		}
		final_str += "\r\n" + body;
		return final_str;
	};

	// std::string const& Message::getBody(void) const
	// {
	// 	return this->_body;
	// };

	// void Message::setBody(std::string const& str)
	// {
	// 	_body += bod;
	// };

	void Message::clear( void )
	{
		method.clear();
		body.clear();
		headers.clear();
		content_length = 0;
	}
}
