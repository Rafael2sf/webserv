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

	void Client::owsTrimmer(std::string& str)
	{
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

	int Client::_updateStatusLine( char const* buff, size_t n )
	{
		std::string str;
		size_t		start;

		req.method.reserve(4);
		req.createMethodVec(std::string().assign(buff, n));
		if (req.method.size() != 3)
		{
			error(400);
			return -1;
		}
		start = req.method[1].find_first_of('?');
		if (start != std::string::npos)
		{
			str = req.method[1].substr(start + 1);
			req.method[1].erase(req.method[1].begin() \
				+ start, req.method[1].end());
			req.method.push_back(str);
		}
		else
			req.method.push_back("");

		for (std::vector<std::string>::iterator it = req.method.begin();
			it != req.method.end(); it++)
		{ std::cout << *it << " "; }
		std::cout << std::endl;

		if (req.getMethodAt(0) != "GET"
			&& req.getMethodAt(0) != "POST"
			&& req.getMethodAt(0) != "DELETE")
		{
			error(405);
			return -1;
		}

		state = HEADER_FIELDS;
		return 0;
	}

	int Client::_updateHeaders( char const* buff, size_t n )
	{
		std::stringstream ss;
		std::string key;
		std::string val;

		ss.str(std::string().assign(buff, n));
		ss.seekg(0);
		ss >> key;
		if (*--key.end() == ':')
			key.erase(--key.end());
		else
		{
			error(400);
			return -1;
		}
		val = ss.str().substr(key.size() + 1, n - key.size() - 1);
		owsTrimmer(val);
		std::transform(key.begin(), key.end(), key.begin(), ::tolower);
		if (key.empty() || val.empty())
		{
			error(400);
			return -1;
		}
		req.setField(key, val);
		std::cout << key << " " << val << std::endl;
		return 0;
	}

	int Client::_updateBody( char const* buff, size_t readval )
	{
		if (req.body.size() == 0)
		{
			if (!req.getField("host"))
			{
				error(400);
				return -1;
			}
			if (req.getField("content-length"))
			{
				size_t content_length = ftStoi(*req.getField("content-length"));
				size_t			max_body_size;
				JSON::Node * 	max_body = 0;

				if (config)
					max_body = config->search(1, "client_max_body_size");
				if (max_body)
					max_body_size = max_body->as<int>();
				else
					max_body_size = 1048576;
				if (max_body_size < content_length)
				{
					error(413);
					return -1;
				}
				req.content_length = content_length;
			}
			else
			{
				state = OK;
				return 0;
			}
		}

		if (req.body.size() + readval < req.content_length)
			req.body.append(buff, readval);
		else
		{
			req.body.append(buff, req.content_length - req.body.size());
			std::cout << '[' << req.body.size() << ']' << std::endl;
			state = OK;
			return readval;
		}
		return readval;
	}

	static void nextField( int *state, char **j )
	{
		if (*state == HEADER_FIELDS
			&& (!strncmp(*j, "\r\n\r\n", 4) || !strncmp(*j, "\n\r\n\r", 4)))
		{
			*state = BODY_CONTENT;
			*j += 4;
		}
		else if (*state == HEADER_FIELDS
			&& !strncmp(*j, "\n\n", 2))
		{
			*state = BODY_CONTENT;
			*j += 2;
		}
		else if (!strncmp(*j, "\r\n", 2) || !strncmp(*j, "\n\r", 2))
			*j += 2;
		else
			*j += 1;
	}

	int Client::update( void )
	{
		std::stringstream	ss;
		ssize_t				readval;
		char				buff[S_BUFFER_SIZE];

		if (state == CONNECTED)
			state = STATUS_LINE;
		while ((readval = recv(fd, buff, S_BUFFER_SIZE - 1, 0)) > 0)
		{
			ssize_t i = 0;
			char * j = 0;
			size_t n = 0;
			buff[readval] = 0;
			while (state == BODY_CONTENT || i < readval)
			{
				if (state == BODY_CONTENT)
				{
					if (_updateBody(buff + i, readval - i) < 0)
						return -1;
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
						if (state == STATUS_LINE)
						{
							if (_updateStatusLine(buff + i, n) < 0)
								return -1;
						}
						else if (_updateHeaders(buff + i, n) < 0)
						{
							return -1;
						}
					}
					else
						state = BODY_CONTENT;
					nextField(&state, &j);
				}
				else
				{
					error(400);
					return -1;
				}
				i += j - (buff + i);
			}
		}
		if (state == STATUS_LINE) // empty request
		{
			error(400);
			return -1;
		}
		return 0;
	}

	void Client::error(int code)
	{
		std::string err;
		
		err = ftItos(code) + ' '+ Server::error[code];
		res.createMethodVec("HTTP/1.1 " + err);
		res.setField("content-type", "text/html");
		res.setField("server", "webserv/0.4");
		res.setField("connection", "close");
		res.body = \
"<html>\n\
<head><title>"+ err + "</title></head>\n\
<body bgcolor=\"white\">\n\
<center><h1>" + err + "</h1></center>\n\
<hr><center>webserv/0.4</center>\n\
</body>\n\
</html>\n";
		res.setField("content-length", ftItos(res.body.size()));
		err = res.toString();
		send(fd, err.c_str(), err.size(), 0);
		return ;
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
		config = 0;
	}
}
