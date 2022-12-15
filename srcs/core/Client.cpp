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

	void Client::_owsTrimmer(std::string& str)
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

	int Client::_validateStatusLine( void )
	{
		int v[2];

		if (req.method[0] != "GET"
			&& req.method[0] != "POST"
			&& req.method[0] != "DELETE")
		{
			error(405, true);
			return -1;
		}

		if (req.method[1][0] != '/')
		{
			error(400, true);
			return -1;
		}

		if (req.method[2].size() != 8
			|| sscanf(req.method[2].c_str(), "HTTP/%d.%d", &v[0], &v[1]) != 2
			|| v[0] == 0)
		{
			error(400, true);
			return -1;
		}

		if (v[0] > 1 || v[1] > 1)
		{
			error(505, true);
			return -1;
		}
		return 0;
	}

	int Client::_updateStatusLine( char const* buff, size_t n )
	{
		std::string str;
		size_t		start;

		req.createMethodVec(std::string().assign(buff, n));
		if (req.method.size() != 3)
		{
			error(400, true);
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

		if (_validateStatusLine() < 0)
			return -1;

		// for (std::vector<std::string>::iterator it = req.method.begin();
		// 	it != req.method.end(); it++)
		// { std::cout << *it << " "; }
		// std::cout << std::endl;

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

		size_t pos = key.find_first_of(':');
		if (pos == std::string::npos)
			return 0;

		if (pos == key.size() - 1)
		{
			key.erase(--key.end());
			val += ss.str().substr(key.size() + 1, n - key.size() - 1);
		}
		else
		{
			val = key.substr(pos + 1);
			key.erase(key.begin() + pos, key.end());
		}
		_owsTrimmer(val);
		std::transform(key.begin(), key.end(), key.begin(), ::tolower);
		if (val.empty() && key == "host")
		{
			error(400, true);
			return -1;
		}
		else if (key.empty())
			return 0;

		if (req.getField(key))
			*req.getField(key) += ',' + val;
		else
			req.setField(key, val);
		//std::cout << key << " " << val << std::endl;
		return 0;
	}

	static JSON::Node *matchLocation(JSON::Node *serv, std::string const &path)
	{
		JSON::Object *tmp;
		size_t last_len = 0;
		JSON::Node *last_match = 0;
		size_t i = 0;

		tmp = dynamic_cast<JSON::Object *>(serv->search(1, "location"));
		if (!tmp)
		{
			DEBUG2("no location");
			return 0;
		}
		for (JSON::Node::iterator loc = tmp->begin(); loc != tmp->end(); loc.skip())
		{
			DEBUG2("LOOP");
			if (path.size() == 1)
				i = path.find_first_of(*path.c_str());
			else
				i = path.find(loc->getProperty());
			if (i != std::string::npos && i == 0)
			{
				i = loc->getProperty().size();
				if (path.size() == i)
					return &*loc;
				if (i > last_len)
				{
					last_len = i;
					last_match = &*loc;
				}
			}
		}
		DEBUG2("END");
		return last_match;
	}

	static bool isMethodAllowed( std::string const& method, JSON::Node const*nptr )
	{
		JSON::Node *p;

		p = nptr->search(1, "limit_except");
		if (!p)
			return true;
		for (JSON::Node::iterator it = p->begin(); it != p->end(); it.skip())
		{
			if (it->type() == JSON::string
				&& method == it->as<std::string const&>())
			{
				return true;
			}
		}
		return false;
	}

	int Client::_peekHeaderFields( void )
	{
		int				size;
		JSON::Node * 	nptr = 0;
	
		if (server)
		{
			location = matchLocation(this->server, req.getMethod()[1]);
			if (!location)
				DEBUG2("HOW");
			DEBUG2("LOCATION = " << location);
		}
		else
			DEBUG2("WTF");
		if (req.getField("host") == 0)
		{
			error(400, true);
			return -1;
		}
		if (!isMethodAllowed(req.method[0], location))
		{
			error(405, true);
			return -1;
		}
		if (req.method[0] != "GET" && req.getField("content-length"))
		{

			if (server)
				nptr = server->search(1, "client_max_body_size");
			if (nptr)
				size = nptr->as<int>();
			else
				size = 1048576;
			req.content_length = stoi(*req.getField("content-length"), std::dec);
			if (size < 0 || size < req.content_length)
			{
				error(413, true);
				return -1;
			}
			req.content_length = req.content_length;
		}
		else
			state = OK;
		return 0;
	}

	int Client::_updateBody( char const* buff, size_t readval )
	{
		if (req.body.size() == 0 && _peekHeaderFields() < 0) // run once
			return -1;
		if (req.body.size() + readval < (size_t)req.content_length)
			req.body.append(buff, readval);
		else
		{
			req.body.append(buff, req.content_length - req.body.size());
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
					return _updateBody(buff + i, readval - i);
				j = std::find(buff + i, buff + readval, '\n');
				if (j != (buff + readval))
				{
					if (j != (buff + i) && *(j - 1) == '\r')
						j--;
					n = j - (buff + i);
					if (n > 0)
					{
						if (state == STATUS_LINE
							&& _updateStatusLine(buff + i, n) < 0)
							return -1;
						else if (_updateHeaders(buff + i, n) < 0)
							return -1;
					}
					else if (state == HEADER_FIELDS)
						state = BODY_CONTENT;
					nextField(&state, &j);
				}
				else
				{
					error(400, true);
					return -1;
				}
				i += j - (buff + i);
			}
		}
		if (readval == 0 && state == STATUS_LINE) // empty request 
		{
			error(400, true);
			return -1;
		}
		return 0; // If EPOLLOUT event, it will be ignored!
	}

	void Client::print_message( Message const& m, std::string const& s  )
	{
		DEBUG(
			std::cerr << std::endl << s << std::endl;
			unsigned int port = htonl(ai.sin_addr.s_addr);
			std::cerr << "[FROM " \
				<< ((port & 0xff000000) >> 24) << '.' \
				<< ((port & 0x00ff0000) >> 16) << '.' \
				<< ((port & 0x0000ff00) >> 8) << '.' \
				<< (port & 0x000000ff) << ':' << \
				htons(ai.sin_port) << ']'<< std::endl;
			for (std::vector<std::string>::const_iterator it = m.method.begin();
				it != m.method.end(); it++)
			{ std::cerr << *it << " "; }
			std::cerr << std::endl;
			for (std::map<std::string, std::string>::const_iterator it = m.headers.begin();
				it != m.headers.end(); it++)
			{ std::cerr << it->first << ": " << it->second << std::endl; }
			std::cerr << '[' << m.body.size() << ']' << std::endl;
		);
	}

	void Client::error(int code, bool close_connection)
	{
		std::string s;

		(void) close_connection;
		s = itos(code, std::dec) + ' ' + Server::error[code];
		res.clear();
		res.createMethodVec("HTTP/1.1 " + s);

		if (close_connection || (req.getField("connection") &&
			*req.getField("connection") == "close"))
			res.setField("connection", "close");
		else
		res.setField("connection", "keep-alive");
		res.body = \
"<html>\n\
<head><title>"+ s + "</title></head>\n\
<body bgcolor=\"white\">\n\
<center><h1>" + s + "</h1></center>\n\
<hr><center>webserv/0.4</center>\n\
</body>\n\
</html>\n";
		res.setField("content-type", "text/html");
		res.setField("content-length", itos(res.body.size(), std::dec));
		s = res.toString();
		send(fd, s.c_str(), s.size(), 0);
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
		server = 0;
	}
}
