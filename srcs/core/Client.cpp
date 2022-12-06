#include "Client.hpp"
#include <fcntl.h>
#include <iomanip>

namespace HTTP
{
	Client::~Client( void )
	{}

	Client::Client( void )
	: state(STATUS_LINE)
	{}

	int Message::_updateStatusLine( std::stringstream & ss, size_t n )
	{
		std::string str;
		size_t		start;
		size_t		end;

		(void) n;
		method.resize(5);
		ss >> method[0]; // Verify valid method - case sensitive
		ss >> method[1]; // Veriy valid path
		ss >> method[2]; // Verify valid protocl/version rfc 2.5 + case_sensitive
		// method[4] -> fragment ?

		start = method[1].find_first_of('?');
		if (start != std::string::npos)
			start++;
		end = method[1].find_first_of('#');
		if (end != std::string::npos)
			end--;

		if (start != std::string::npos && start < end)
			method[3] = method[1].substr(start, end - start + 1);
		if (end != std::string::npos)
			method[4] = method[1].substr(end + 1);
		if (start != std::string::npos && start < end)
			method[1].erase(start - 1);
		else if (end != std::string::npos)
			method[1].erase(end + 1);

		std::cout
			<< method[0] << " " \
			<< method[1] << " " \
			<< method[2] << " " \
			<< method[3] << " " \
			<< method[4] << std::endl;
		return 0;
	}

	int Message::_updateHeaders( std::stringstream & ss, size_t n )
	{
		std::string key;
		std::string val;
		std::string str;

		ss >> key;
		if (*--key.end() == ':')
			key.erase(--key.end());
		else
			return -1;
		val = wp_trimmer(ss.str().substr(key.size() + 1, n - key.size() - 1));
		if (val.empty())
			return -1;
		std::transform(key.begin(), key.end(), key.begin(), ::tolower);
		std::cout << "[FIELD] " << key << " : " << val << std::endl;
		this->add(key, val);
		return 0;
	}

	int Message::_updateBody( char const * buff, size_t readval, size_t content_length )
	{
		if (_body.size() + readval <= content_length)
			_body += buff;
		else
		{
			_body += std::string(buff, content_length - _body.size());
			return 0;
		}
		return readval;
	}

	int Client::update( void )
	{
		std::stringstream	ss;
		ssize_t				readval;
		char				buff[TMP_BUFF];

		while ((readval = recv(fd, buff, TMP_BUFF - 1, 0)) > 0)
		{
			ssize_t i = 0;
			char * j = 0;
			size_t n = 0;
			buff[readval] = 0;
			DEBUG2("RECEIVED = " << buff);
			while (i < readval)
			{
				if (state == BODY_CONTENT)
				{
					size_t content_length = ftStoi(req.get_head_val("content-length"));
					if (!content_length || req._updateBody(buff + i,
						(buff + readval) - (buff + i) + 1, content_length) == 0)
					{
						std::cout << req._body << std::endl;
						DEBUG2("PARSE SUCCESS");
						state = OK;
						return 0;
					}
					break ;
				}
				j = std::find(buff + i, buff + readval, '\n');
				if (j != (buff + readval))
				{
					if (*(j - 1) == '\r')
						j--;

					n = j - (buff + i);
					if (n > 0)
					{
						std::cout << n << std::endl;
						ss.str(std::string(buff + i, n));
						ss.seekg(0);

						if (state == STATUS_LINE)
						{
							if (req._updateStatusLine(ss, n) == -1)
							{
								DEBUG2("exit 2");
								return -1;
							}
							state = HEADER_FIELDS;
						}
						else if(req._updateHeaders(ss, n) == -1)
						{
							DEBUG2("exit 1");
							return -1;
						}
					}
					else
						state = BODY_CONTENT;
					if (state == HEADER_FIELDS
						&& !strncmp(j, "\r\n\r\n", 4))
					{
						state = BODY_CONTENT;
						j += 4;
					}
					else if (state == HEADER_FIELDS
						&& !strncmp(j, "\n\n", 2))
					{
						state = BODY_CONTENT;
						j += 2;
					}
					else if (!strncmp(j, "\r\n", 2))
						j += 2;
					else
						j++;
				}
				else
					return -1;
				i += j - (buff + i);
			}
		}
		DEBUG2("read = " << read(fd, buff, TMP_BUFF));
		return 0;
	}

	bool Client::ok( void )
	{
		return state == OK;
	}
}
