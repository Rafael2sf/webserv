#include "Client.hpp"
#include <fcntl.h>
#include <iomanip>
#include "Node.hpp"
#include "Server.hpp"

namespace HTTP
{
	Client::~Client( void )
	{}

	Client::Client( void )
	: state(CONNECTED)
	{}

	Client::Client( Client const& other )
	: state(CONNECTED)
	{
		(void) other;
	}

	Client & Client::operator=( Client const& rhs )
	{
		DEBUG2("DO NOT CALL THIS COPY OPERATOR");
		(void) rhs;
		return *this;
	}

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
		val = ss.str().substr(key.size() + 1, n - key.size() - 1);
		owsTrimmer(val);
		if (val.empty())
			return -1;
		std::transform(key.begin(), key.end(), key.begin(), ::tolower);
		//DEBUG2("[FIELD] " << key << " : " << val);
		this->setField(key, val);
		return 0;
	}

	int Message::_updateBody( char const * buff, size_t readval, size_t content_length )
	{
		if (body.size() + readval < content_length)
			body.append(buff, readval);
		else
		{
			body += std::string(buff, content_length - body.size());
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
		char				buff[S_BUFFER_SIZE];

		if (state == CONNECTED)
			state = STATUS_LINE;
		if ((readval = recv(fd, buff, S_BUFFER_SIZE - 1, 0)) > 0)
		{
			ssize_t i = 0;
			char * j = 0;
			size_t n = 0;
			buff[readval] = 0;
			timestamp = time(NULL);
			while (state == BODY_CONTENT || i < readval)
			{
				if (state == BODY_CONTENT)
				{
					size_t content_length = 0;
					if (req.getField("content-length"))
						content_length = ftStoi(*req.getField("content-length"));
					if (content_length && req.body.size() == 0)
					{
						size_t			max_body_size;
						JSON::Node * 	max_body = 0;
						if (config)
							max_body = config->search(1, "client_max_body_size");
						if (max_body)
						{
							max_body_size = max_body->as<int>();
						}
						else
							max_body_size = 1048576;
						if (max_body_size < content_length)
						{
							DEBUG2("content-length exceeds max_client_body_size: " << max_body_size);
							error(413);
							return -1;
						}
					}
					if (!content_length || req._updateBody(buff + i,
						(buff + readval) - (buff + i), content_length) == 0)
					{
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
							{
								error(400);
								return -1;
							}
							state = HEADER_FIELDS;
						}
						else if (req._updateHeaders(ss, n) == -1)
						{
							error(400);
							return -1;
						}
					}
					else
						state = BODY_CONTENT;
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
				{
					error(400);
					return -1;
				}
				i += j - (buff + i);
			}
		}
		if (readval == -1)			//Tried to read an EPOLLOUT!
			return 0;
		if (state == STATUS_LINE)
		{
			error(400);
			return -1;
		}
		return 0;
	}

	void Client::error(int code)
	{
		std::string str;

		res.createMethodVec("HTTP/1.1 " + ftItos(code) + Server::error[code]);
		res.setField("content-type", "text/html");
		res.setField("content-length", "12");
		res.body = "<h1>" + ftItos(code) + "</h1>";
		str = res.toString();
		send(fd, str.c_str(), str.size(), 0);
		return ;
	}

	bool Client::ok( void )
	{
		return state == OK;
	}

	void Client::setOk( void )
	{
		state = OK;
	}

	bool Client::sending( void ) 
	{
		return state == SENDING;
	}

	void Client::setSending( void )
	{
		state = SENDING;
	}
	
	void Client::reset( void )
	{
		req.clear();
		res.clear();
		state = CONNECTED;
		config = 0;
	}
}
