#include "Client.hpp"
#include <fcntl.h>
#include <iomanip>
#include "Node.hpp"

namespace HTTP
{
	Client::~Client( void )
	{}

	Client::Client( void )
	: state(CONNECTED)
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

		// DEBUG2(method[0] << " "
		// 	<< method[1] << " "
		// 	<< method[2] << " "
		// 	<< method[3] << " "
		// 	<< method[4]);
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
		//DEBUG2("[FIELD] " << key << " : " << val);
		this->add(key, val);
		return 0;
	}

	int Message::_updateBody( char const * buff, size_t readval, size_t content_length )
	{
		if (_body.size() + readval < content_length)
			_body.append(buff, readval);
		else
		{
			_body += std::string(buff, content_length - _body.size());
			//DEBUG2("BODY OK [" << readval << "] [" << content_length << "] [" << _body.size() << ']');
			return 0;
		}
		//DEBUG2("BODY UPDATE [" << readval << "] [" << content_length << "] [" << _body.size() << ']');
		return readval;
	}

	int Client::update( void )
	{
		std::stringstream	ss;
		ssize_t				readval;
		char				buff[TMP_BUFF];

		if (state == CONNECTED)
			state = STATUS_LINE;
		if ((readval = recv(fd, buff, TMP_BUFF - 1, 0)) > 0)
		{
			ssize_t i = 0;
			char * j = 0;
			size_t n = 0;
			buff[readval] = 0;
			//DEBUG2('[' << readval << "] " << "RECEIVED = " << buff);
			while (state == BODY_CONTENT || i < readval)
			{
				if (state == BODY_CONTENT)
				{
					size_t content_length = ftStoi(req.get_head_val("content-length"));
					if (content_length && req._body.size() == 0)
					{
						size_t			max_body_size;
						JSON::Node * 	max_body = 0;
						if (req.conf)
							max_body = req.conf->search(1, "client_max_body_size");
						if (max_body)
						{
							max_body_size = max_body->as<int>();
						}
						else
							max_body_size = 1048576;
						if (max_body_size < content_length)
						{
							DEBUG2("content-length exceeds max_client_body_size: " << max_body_size);
							// send(fd, "413", 3, 0);
							return -1;
						}
						
					}
					//DEBUG2("CONTENT_LENGTH = " << content_length);
					if (!content_length || req._updateBody(buff + i,
						(buff + readval) - (buff + i), content_length) == 0)
					{
						//std::cout << req._body << std::endl;
						//DEBUG2("PARSE SUCCESS");
						state = OK;
						return 0;
					}
					break ;
				}
				j = std::find(buff + i, buff + readval, '\n');
				if (j != (buff + readval))
				{
					if (j != (buff + i) && *(j - 1) == '\r')
						j--;

					n = j - (buff + i);
					if (n > 0)
					{
						ss.str(std::string(buff + i, n));
						ss.seekg(0);

						if (state == STATUS_LINE)
						{
							if (req._updateStatusLine(ss, n) == -1)
								return -1;
							state = HEADER_FIELDS;
						}
						else if (req._updateHeaders(ss, n) == -1)
							return -1;
					}
					else
					{
						//DEBUG2("BODY");
						state = BODY_CONTENT;
					}
					if (state == HEADER_FIELDS
						&& (!strncmp(j, "\r\n\r\n", 4) || !strncmp(j, "\n\r\n\r", 4)))
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
					else if (!strncmp(j, "\r\n", 2) || !strncmp(j, "\n\r", 2))
						j += 2;
					else
						j++;
				}
				else
					return -1;
				i += j - (buff + i);
			}
		}
		if (state == STATUS_LINE)
			return -1;
		return 0;
	}

	bool Client::ok( void )
	{
		return state == OK;
	}

	void Client::reset( void )
	{
		req.clear();
		res.clear();
		state = CONNECTED;
	}
}
