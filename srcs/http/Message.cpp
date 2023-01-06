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
			return ;
		}
		headers.insert(std::make_pair(key, value));
	};

	std::string Message::toString(void)
	{
		std::string	final_str;

		setField("server", "Webserv/1.0");
		setField("date", getDate(time(0)));
		for (std::vector<std::string>::iterator it =
			method.begin(); it != method.end(); it++)
		{
			if (!it->empty())
				final_str += *it + ' ';
		}
		if (final_str.size() != 0)
			final_str.replace(final_str.size() - 1, 1, "\r\n");
		for (std::map<std::string, std::string>::iterator it =
			headers.begin(); it != headers.end(); it++)
		{
			final_str += it->first + ": " + it->second + "\r\n";
		}
		final_str += "\r\n" + body;
		return final_str;
	};

	void Message::clear( void )
	{
		method.clear();
		body.clear();
		headers.clear();
		header_bytes = 0;
		content_length = 0;
	}
}
