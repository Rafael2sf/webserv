#include "Client.hpp"
#include <fcntl.h>
#include <iomanip>
#include "Node.hpp"
#include "Server.hpp"
#include <algorithm>

namespace HTTP
{
	Client::~Client( void )
	{
		reset();
	}

	Client::Client( void )
	: fd(-1), timestamp(0), server(0), location(0), 
	cgiSentBytes(0), childPid(0), state(CONNECTED), fp(NULL)
	{
		clientPipe[0] = 0;
		clientPipe[1] = 0;
		memset(&ai, 0, sizeof(ai));
	}

	Client::Client( Client const& other )
	: fd(-1), timestamp(0), server(0), location(0),
	cgiSentBytes(0), childPid(0), state(CONNECTED), fp(NULL)
	{
		(void) other;
		clientPipe[0] = 0;
		clientPipe[1] = 0;
		memset(&ai, 0, sizeof(ai));
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

	static int validUrl(std::string & s)
	{
		int depth = 0;
		size_t i = 0;

		if (s[0] != '/')
			return -1;
		while (i < s.size())
		{
			i = s.find_first_of('/', i);
			if ((i + 1) == s.size() || i == std::string::npos)
				break ;
			if (s[++i] == '/')
			{
				s.erase(i--, 1);
				continue ;
			}

			if (!s.compare(i, 3, ".."))
			{
				s.push_back('/');
				depth--;
			}
			else if (!s.compare(i, 3, "../"))
				depth--;
			else if (!s.compare(i, 2, "."))
				s.erase(i--, 1);
			else if (s.compare(i, 2, "./") != 0)
				depth++;

			if (depth < 0)
				return -1;
		}
		return 0;
	}

	int Client::_validateRequestLine( void )
	{
		int v[2];

		for (std::string::iterator it = req.method[0].begin();
			it != req.method[0].end(); it++)
		{
			if (!isupper(*it))
			{
				error(400, true);
				return -1;
			}
		}
		if (validUrl(req.method[1]) < 0)
		{
			error(400, true);
			return -1;
		}

		if (req.method[2] != "")
		{
			if (req.method[2].size() != 8 || sscanf(req.method[2].c_str(),
				"HTTP/%d.%d", &v[0], &v[1]) != 2)
			{
				error(400, true);
				return -1;
			}

			if (v[0] != 1 || v[1] != 1)
			{
				error(505, true);
				return -1;
			}
		}
		return 0;
	}

	int Client::_getHostFromUrl( void )
	{
		DEBUG2("HERE!");
		if (req.method[1][0] == '/')
			return 0;
		size_t x = req.method[1].find("http://");
		if (x != 0)
		{
			x = req.method[1].find("https://");
			if (x == 0)
				x += 8;
			else
				return -1;
		}
		else
			x += 7;
		DEBUG2("(X) " << x);
		std::string::iterator y = std::find(
			req.method[1].begin() + x, req.method[1].end(), '/');
		if (y == req.method[1].end())
			return -1;
		std::string host = req.method[1].substr(x, std::distance(req.method[1].begin() + x, y));
		req.setField("host", req.method[1].substr(x, std::distance(req.method[1].begin() + x, y)));
		req.method[1].erase(req.method[1].begin(), y);
		if (req.getField("host")->empty() || req.method[1].empty())
			return -1;
		return 0;
	}

	int Client::_checkSpacesRequestLine( char const* buff, size_t n )
	{
		size_t index;

		if (isspace(*buff))
			return -1;
		index = req.method[0].size();
		if (buff[index] != ' ' || isspace(buff[index + 1]))
			return -1;
		index += 1 + req.method[1].size();
		if (buff[index] != ' ' || isspace(buff[index + 1]))
			return -1;
		index += 1 + req.method[2].size();
		if (index != n || (index + 1 == n && buff[index + 1] != '\r'))
			return -1;
		return 0;
	}

	int Client::_updateRequestLine( char const* buff, size_t n )
	{
		std::string str;
		size_t		start;

		req.createMethodVec(std::string().assign(buff, n));
		if (req.method.size() != 3
			|| _checkSpacesRequestLine(buff, n) < 0)
		{
			error(400, true);
			return -1;
		}
		if (req.method[1].size() > S_URI_MAX)
		{
			error(414, true);
			return -1;
		}
		if (_getHostFromUrl() < 0)
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
		if (_validateRequestLine() < 0)
			return -1;
		state = HEADER_FIELDS;
		return 0;
	}

	int Client::_updateHeaders( char const* buff, size_t n )
	{
		std::stringstream ss;
		std::string key;
		std::string val;

		req.header_bytes += n;
		if (n > S_FIELD_MAX
			|| req.header_bytes > S_HEADERS_MAX)
		{
			error(431, true);
			return -1;
		}
		if (isspace(*buff))
		{
			error(400, true);
			return -1;
		}
		ss.str(std::string().assign(buff, n));
		ss.seekg(0);
		ss >> key;

		size_t pos = key.find_first_of(':');
		if (pos == std::string::npos)
		{
			error(400, true);
			return -1;
		}
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
		if (key.empty())
			return 0;
		_owsTrimmer(val);
		if (!val.empty() && (*val.begin() == '\r' || *--val.end() == '\r'))
		{
			error(400, true);
			return -1;
		}
		std::transform(key.begin(), key.end(), key.begin(), ::tolower);
		if (key == "host" && (val.empty() || req.getField("host")))
		{
			error(400, true);
			return -1;
		}
		if (req.getField(key) && !req.getField(key)->empty())
			*req.getField(key) += ", " + val;
		else
			req.setField(key, val);
		return 0;
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

	void Client::_defaultPage( int code, bool close_connection )
	{
		std::string s;

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
	}

	// if adress empty use config
	void Client::_redirect( std::string const& address )
	{
		JSON::Node * loc;
		int code;

		if (address.empty())
		{
			loc = location->search(1, "redirect");
			code = loc->begin()->as<int>();
			std::string const& page = (++loc->begin())->as<std::string const&>();
			_defaultPage(code, false);
			res.setField("location", page);
		}
		else
		{
			_defaultPage(301, false);
			res.setField("location", address);
		}
		state = REDIRECT;
	}

	int Client::_peekHeaderFields( Sockets const& sockets )
	{
		int				size;
		JSON::Node * 	nptr = 0;

		if (req.getField("host") == 0)
		{
			error(400, true);
			return -1;
		}
		server = matchServer(sockets, *this);
		if (server)
			location = matchLocation(server, req.method[1]);
		if (!location)
		{
			error(404, false);
			return -1;
		}
		if (!isMethodAllowed(req.method[0], location))
		{
			error(405, true);
			return -1;
		}
		if (location->search(1, "redirect"))
		{
			_redirect("");
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
			if (size < 0 || req.content_length > (size_t)size)
			{
				error(413, true);
				return -1;
			}
			req.content_length = stoi(*req.getField("content-length"), std::dec);
		}
		else
			state = OK;
		return 0;
	}

	int Client::_updateBody( char const* buff, size_t readval, Sockets const& sockets )
	{
		if (req.body.size() == 0 && _peekHeaderFields(sockets) < 0)
		{
			if (state == REDIRECT)
				return 0;
			return -1;
		}
		if (req.body.size() + readval < (size_t)req.content_length)
		{
			if (req.getField("content-type")
				&& *req.getField("content-type") == "multipart/form-data")
				state = OK;
			req.body.append(buff, readval);
		}
		else
		{
			req.body.append(buff, req.content_length - req.body.size());
			state = OK;
			return readval;
		}
		return readval;
	}

	int Client::update( Sockets const& sockets )
	{
		char	buff[S_BUFFER_SIZE];
		ssize_t	readval;
		ssize_t	i = 0, n = 0;
		char *	j = 0;

		if (state == CONNECTED)
			state = STATUS_LINE;
		if ((readval = recv(fd, buff, S_BUFFER_SIZE - 1, 0)) > 0)
		{
			buff[readval] = 0;
			timestamp = time(NULL);
			if (state == CGI_PIPING)
			{
				if (req.body.empty())
					req.body.append(buff, readval);
				state = OK;
				return 0;
			}
			while (state == BODY_CONTENT || i < readval)
			{
				if (state == BODY_CONTENT)
					return _updateBody(buff + i, readval - i, sockets);
				j = std::find(buff + i, buff + readval, '\n');
				if (j != (buff + readval))
				{
					if (j != (buff + i) && *(j - 1) == '\r')
						j--;
					else
					{
						error(400, true);
						return -1;
					}
					n = j - (buff + i);
					if (n > 0)
					{
						if (state == STATUS_LINE)
						{
							if (_updateRequestLine(buff + i, n) < 0)
								return -1;
						}
						else if (_updateHeaders(buff + i, n) < 0)
							return -1;
					}
					else if (state == HEADER_FIELDS)
						state = BODY_CONTENT;
					else if (state != STATUS_LINE)
					{
						error(400, true);
						return -1;
					}
					if (!strncmp(j, "\r\n", 2))
						j += 2;
					else
					{
						error(400, true);
						return -1;
					}
				}
				else
				{
					error(400, true);
					return -1;
				}
				i += j - (buff + i);
			}
		}
		if (readval == 0 && state == STATUS_LINE)
		{
			error(400, true);
			return -1;
		}
		return 0;
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

	std::string const* Client::_errorPage( int code )
	{
		JSON::Node * nptr = server->search(1, "error_page");

		if (!nptr)
			return 0;
		for (JSON::Node::const_iterator it = nptr->begin();
			it != nptr->end(); it.skip())
		{
			for (JSON::Node::const_iterator err = it->begin();
				err != it->end(); err++)
			{
				if (err->type() == JSON::integer && 
					err->as<int>() == code)
				{
					return &it->getProperty();
				}
			}
		}
		return 0;
	}

	void Client::error(int code, bool close_connection)
	{
		std::string s;
		std::string const * ep = 0;

		if (server)
			ep = _errorPage(code);
		if (ep)
			code = 301;
		_defaultPage(code, close_connection);
		if (ep)
			res.setField("location", *ep);
		s = res.toString();
		if (send(fd, s.c_str(), s.size(), 0) == -1)
			res.setField("connection", "close");
		return ;
	}

	void Client::reset( void )
	{
		req.clear();
		res.clear();
		state = CONNECTED;
		server = 0;
		location = 0;
		cgiSentBytes = 0;
		childPid = 0;
		if (fp != NULL)
			fclose(fp);
		fp = NULL;
		if (clientPipe[0] != 0)
			close(clientPipe[0]);
		clientPipe[0] = 0;
		if (clientPipe[1] != 0)
			close(clientPipe[1]);
		clientPipe[1] = 0;
	}

	static int fopenr(FILE **fp, std::string const& path)
	{
		struct stat		stat;

		*fp = fopen(path.c_str(), "r");
		if (*fp == NULL)
		{
			if (errno == ENOENT || errno == ENOTDIR)
				return 404;
			return 403;
		}
		if (lstat(path.c_str(), &stat) == -1) 
		{
			fclose(*fp);
			*fp = NULL;
			return 404;
		}
		if (S_ISDIR(stat.st_mode))
			return 1;
		return 0;
	}

	/*
		TODO: HANLED CGI AS INdex
		-1 (something hapened response is built stop)
		0 (success found file)
		404 (no such file)
	*/
	int Client::tryIndex(std::string const& index, std::string & path)
	{
		std::string		path_index;
		JSON::Node *	root;
		JSON::Node *	loc;
		//bool			redirect = false;
		int				code;

		DEBUG2("INDEX>" << index);
		path_index = path + index;
		loc = matchLocation(server, path_index);
		if (!loc)
		{
			error(404, false);
			return -1;
		}

		// new root and index
		root = loc->search(1, "root");
		if (!root)
			path_index = std::string("./html/");
		else
		{
			path_index = root->as<std::string const&>();
			DEBUG2("ROOT> " << path_index);
		}
		if (*--path_index.end() == '/')
			path_index.erase(--path_index.end());
		path_index += req.getMethod()[1] + index;

		DEBUG2("TRYING> " << path_index);
		code = fopenr(&fp, path_index);
		if (code == 404)
		{
			DEBUG2("HOW!?");
			return code;
		}
		if (code == 1)
		{
			fclose(fp);
			fp = NULL;
			if (*--path_index.end() != '/')
			{
				_redirect(req.method[1] + index + '/');
				return -1;
			}
			DEBUG2("INDEXING> " << path_index);
			dirIndex(path_index);
			return -1;
		}
		else if (code != 0)
			return code;
		path.swap(path_index);
		return 0;
	}

	bool Client::getFile(std::string & path)
	{
		int				code;
		JSON::Node *	var = 0;

		code = fopenr(&fp, path);
		if (code == 1)// dir
		{
			if (*--path.end() != '/')
			{
				error(403, false);
				return false;
			}
			var = location->search(1, "index");
			if (var)
			{
				fclose(fp);
				fp = NULL;
				for (JSON::Node::const_iterator it = var->begin();
					it != var->end(); it++)
				{
					code = tryIndex(it->as<std::string const&>(), path);
					switch (code)
					{
						case 404:
							continue ;
						case -1:
							return false;
						case 0:
							return true;
						default:
							error(403, false);
							return false;
					}
				}
			}
			else
			{
					code = tryIndex("index.html", path);
					switch (code)
					{
						case 404:
							break ;
						case -1:
							return false;
						case 0:
							return true;
						default:
							error(403, false);
							return false;
					}
			}
			DEBUG2("OK> " << path);
			dirIndex(path);
			return false;
		}
		else if (code != 0)
		{
			error(code, false);
			return false;
		}
		return true;
	}

	int Client::contentEncoding(void) 
	{
		static char			buf[S_BUFFER_SIZE + 1];
		int					read_nbr = 0;
		std::string			str;

		//memset(buf, 0, S_BUFFER_SIZE);
		if (state == REDIRECT)
		{
			str = res.toString();
			if (send(fd, str.c_str(), str.size(), 0) == -1)
			{
				error(500, true);
				return -1;
			}
			return 0;
		}
		read_nbr = fread(buf, 1, S_BUFFER_SIZE, fp);
		if (read_nbr == -1)
		{
			error(500, true);
			return -1;
		}
		buf[read_nbr] = 0;
		if (state == OK) 
		{
			if (read_nbr < S_BUFFER_SIZE) //Good also for read of 0
			{
				res.setField("content-length", itos(read_nbr, std::dec));
				res.body.assign(buf, read_nbr);
				str = res.toString();
				if (send(fd, str.c_str(), str.size(), 0) == -1)
				{
					error(500, true);
					return -1;
				}
				return 0;
			}
			res.setField("transfer-encoding", "chunked");
			str = res.toString();
			state = SENDING;
		}
		if (read_nbr != 0) {
			str += itos(read_nbr, std::hex) + "\r\n";
			res.body.assign(buf, read_nbr);
			str += res.body + "\r\n";
			if (send(fd, str.c_str(), str.size(), 0) == -1)
			{
				state = OK;
				error(500, true);
				return -1;
			}
			res.body.clear();
		}
		else
		{
			if (send(fd, "0\r\n\r\n", 5, 0) == -1)
			{
				error(500, true);
				return -1;
			}
			fclose(fp);
			fp = NULL;
		}
		return read_nbr;
	};

	void Client::dirIndex(std::string const& path)
	{
		JSON::Node * autoindex;
		std::string	 str;

		if (!location)
			return error(404, false);
		res.createMethodVec("HTTP/1.1 200 OK");
		res.setField("content-type", "text/html");

		autoindex = location->search(1, "autoindex");
		if (!autoindex || !autoindex->as<bool>())
			return error(404, false);

		DIR * dirp = opendir(path.c_str());
		if (!dirp)
			return error(403, false);
		res.body = "<html>\n<head>file explorer</head>\n<body>\n<hr><pre><a href=\"../\"/>../</a>\n";
		dirent * dp;
		while ((dp = readdir(dirp)) != NULL)
		{
			struct stat stat;
			if (dp->d_name[0] != '.')
			{
				if (lstat(std::string(path.c_str() + std::string(dp->d_name)).c_str(), &stat) != -1
					&& S_ISDIR(stat.st_mode))
				{
					res.body += "<a href=\"" + std::string(dp->d_name) \
					+ "/\"/>" + std::string(dp->d_name) + "/</a>\n";
				}
				else
				{
					res.body += "<a href=\"" + std::string(dp->d_name) \
					+ "\"/>" + std::string(dp->d_name) + "</a>\n";
				}
			}
		}
		closedir(dirp);
		res.body += "</hr></pre>\n</body>\n</html>\n";
		res.setField("content-length", itos(res.body.length(), std::dec));
		if (req.getField("connection")
			&& *req.getField("connection") == "close")
			res.setField("connection", "close");
		else
			res.setField("connection", "keep-alive");
		str = res.toString();
		if (send(fd, str.c_str(), str.size(), 0) == -1)
			return error(500, true);			//??????????
		return ;
	};
}
