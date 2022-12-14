#include "Server.hpp"
#include "Post.hpp"
#include "Get.hpp"
#include "Delete.hpp"

namespace HTTP
{
	int Server::state = 0;
	std::map<std::string, std::string>	
		Server::mime = std::map<std::string, std::string>();
	std::map<int, std::string>
		Server::error = std::map<int, std::string>();

	Server::~Server(void)
	{
	}

	Server::Server(void)
	{
	}

	Server::Server(Server const &other)
	{
		*this = other;
	}

	Server &
	Server::operator=(Server const &rhs)
	{
		DEBUG2("DO NOT CALL THIS COPY OPERATOR");
		(void)rhs;
		return *this;
	}

	int
	Server::init(void)
	{
		DEBUG2("server default init");
		return 0;
	}

	static int stohp_validate(char const *s)
	{
		int i = 0;

		if (strchr(s, '.'))
		{
			for (int j = 0; j < 4; j++)
			{
				i = 0;
				while (isdigit(s[i]))
					i++;
				if (i <= 0 || i > 3)
					return -1;
				if (j != 3 && s[i] != '.')
					return -1;
				if (j == 3)
					s += i;
				else
					s += i + 1;
			}
			if (s[i] == ':')
				i++;
		}
		else if (!strncmp(s, "localhost", 9))
		{
			i += 9;
			if (s[i] == ':')
			{
				if (!isdigit(s[i + 1]))
					return -1;
				i++;
			}
			else if (s[i])
				return -1;
		}
		while (isdigit(s[i]))
			i++;
		if (s[i])
			return -1;
		return 0;
	}

	int stohp(char const *s, unsigned int *host, unsigned int *port)
	{
		int tmp[4];

		*host = 0;
		*port = 0;
		memset(tmp, 0, sizeof(int) * 4);
		if (stohp_validate(s) == -1)
			return -1;
		if (strchr(s, '.'))
		{
			if (strchr(s, ':'))
			{
				if (sscanf(s, "%d.%d.%d.%d:%d",
						   &tmp[0], &tmp[1], &tmp[2], &tmp[3], port) != 5)
					return -1;
			}
			else
			{
				*port = 8000;
				if (sscanf(s, "%d.%d.%d.%d",
						   &tmp[0], &tmp[1], &tmp[2], &tmp[3]) != 4)
					return -1;
			}
		}
		else
		{
			if (!strncmp(s, "localhost", 9))
			{
				tmp[0] = 127;
				tmp[3] = 1;
				if (s[9] == ':' && sscanf(s + 10, "%d", port) != 1)
					return -1;
			}
			else if (sscanf(s, "%d", port) != 1)
				return -1;
		}
		if (*port > 65535)
			return -1;
		for (int i = 0; i < 4; i++)
		{
			if (tmp[i] < 0 || tmp[i] > 255)
				return -1;
			*host += tmp[i];
			if (i != 3)
				*host <<= 8;
		}
		return 0;
	}

	int listenMap(JSON::Json const &json, Sockets &so)
	{
		unsigned int host, port;
		JSON::Node *t;
		std::vector<std::pair<unsigned int, unsigned int> > used;
		t_sock_info * si;

		for (JSON::Node::iterator it = json.tokens->begin();
			 it != json.tokens->end(); it.skip())
		{
			t = it->search(1, "listen");
			if (!t)
			{
				host = 0;
				port = 8000;
				so.insert(host, port)->config = &*it;
			}
			else
			{
				for (JSON::Node::iterator i = t->begin();
					 i != t->end(); i++)
				{
					if (stohp(i->as<std::string const &>().c_str(), &host, &port) == -1)
						return err(-1, "invalid listen field");
					if (std::find(used.begin(), used.end(), std::make_pair(host, port)) != used.end())
						return err(-1, "duplicate listen field");
					si = so.insert(host, port);
					if (si)
					{
						si->config = &*it;
						used.push_back(std::make_pair(host, port));
					}
				}
				used.clear();
			}
		}
		return 0;
	}

	int
	Server::init(char const *filepath)
	{
		if (config.from(filepath) < 0)
		{
			std::cerr << "webserv: error: " \
				<< filepath << ":" << config.err() << std::endl;
			return -1;
		}
		DEBUG(std::cout << "| JSON |\n"; config.cout());
		if (listenMap(config, socks) == -1)
			return -1;
		if (socks.listen() == -1)
			return -1;
		epoll.init(socks);
		_init();
		return (0);
	}

	static JSON::Node *matchCon(Sockets &so, Client const &cli)
	{
		std::list<t_sock_info *> interest;
		bool perfect_match = 0;

		for (std::list<t_sock_info>::iterator it = so.list.begin();
			 it != so.list.end(); it++)
		{
			if ((*it).addr.sin_port == cli.ai.sin_port
				&& (((*it).addr.sin_addr.s_addr == cli.ai.sin_addr.s_addr)
					|| (*it).addr.sin_addr.s_addr == 0))
			{
				if (((*it).addr.sin_addr.s_addr) != 0)
					perfect_match = true;
				interest.push_back(&(*it));
			}
		}
		// if full match remove 0.0.0.0
		std::list<t_sock_info *>::iterator tmp;
		if (perfect_match && cli.ai.sin_addr.s_addr != 0)
		{
			for (std::list<t_sock_info *>::iterator it = interest.begin();
				 it != interest.end(); it++)
			{
				if (((*it)->addr.sin_addr.s_addr) == 0)
				{
					tmp = it++;
					interest.erase(tmp);
					it--;
				}
			}
		}
		if (interest.size() == 0)
			return 0;
		else if (interest.size() == 1)
			return (*interest.begin())->config;
		// TODO -- look at server_name
		return (*interest.begin())->config;
	}

	void
	Server::_accept( t_sock_info const& si )
	{
		int					socket;
		Client * 			cl;

		try
		{
			if ((socket = accept(si.fd, 0, 0)) == -1)
			{
				err(-1);
				return ;
			}
			if (epoll.insert(socket, CLIENT_CONNECT) == -1)
			{
				err(-1);
				return ;
			}
			cl = &clients.insert(clients.end(), 
				std::make_pair(socket, Client()))->second;
			cl->fd = socket;
			cl->ai.sin_addr.s_addr = si.addr.sin_addr.s_addr;
			cl->ai.sin_port = si.addr.sin_port;
			cl->config = 0;
			cl->timestamp = time(NULL);
		}
		catch ( std::exception const& e )
		{
			DEBUG2("failed to create client: " << e.what());
			epoll.erase(socket);
			close(socket);
		}
	}

	void
	Server::_update( int socket )
	{
		Client * cl;

		try
		{
			cl = &clients.at(socket);
			if (cl->ok() || cl->sending())
				_handle(*cl);
			else 
			{
				if (!cl->config)
					cl->config = matchCon(socks, *cl);
				if (cl->update() == -1)
				{
					if (epoll.erase(cl->fd) == -1)
						DEBUG2("epoll.erase() failed");
					clients.erase(cl->fd);
				}
			}
		}
		catch (const std::exception &e)
		{
			cl->error(500);
			epoll.erase(socket);
			clients.erase(socket);
		}
	}

	void
	Server::loop(void)
	{
		t_sock_info *	si;
		int				socket;
		int				events;

		if (socks.list.empty())
			exit(err(1, "logic error", "no sockets available"));
		while (1)
		{
			if (state)
				break ;
			_timeout();
			events = epoll.wait();
			// DEBUG2("Finished sleeping");
			for (int i = 0; i < events; i++)
			{
				socket = epoll[i].data.fd;
				si = socks.find(socket);
				if (si)
					_accept(*si);
				else
					_update(socket);
			}
		}
	}

	void Server::_handle(Client & client)
	{	
		_methodChoice(client);

		DEBUG(
			unsigned int port = htonl(client.ai.sin_addr.s_addr);
			std::cerr << std::endl;
			std::cerr << '[' << client.fd << "] [FROM " \
				<< ((port & 0xff000000) >> 24) << '.' \
				<< ((port & 0x00ff0000) >> 16) << '.' \
				<< ((port & 0x0000ff00) >> 8) << '.' \
				<< (port & 0x000000ff) << ':' << \
				htons(client.ai.sin_port) << "] [" \
				<< client.req.getMethod()[0] << ' ' \
				<< client.req.getMethod()[1] << ']' << std::endl;
		);
		if (client.ok())
		{
			if (client.req.getField("connection") &&
					*client.req.getField("connection") == "close")
			{
				if (epoll.erase(client.fd) == -1)
					DEBUG2("epoll.erase() failed");
				clients.erase(client.fd);
			}
			else
				client.reset();
		}
	}

	void	Server::_timeout(void)
	{
		double seconds = time(NULL);

		for (std::map<int, Client>::iterator it = clients.begin();
			 it != clients.end(); it++)
		{
			if (seconds - it->second.timestamp >= S_CONN_TIMEOUT)
			{
				DEBUG2('[' << it->first << "] timed out");
				if (epoll.erase(it->first) == -1)
					DEBUG2("epoll.erase() failed");
				clients.erase(it->first);
				it = clients.begin();
				if (it == clients.end())
					break;
			}
		}
	}

	void Server::_methodChoice(Client & client) {

		std::string str = client.req.getMethod()[0];
		if (str =="POST")
			Post().response(client);
		else if (str == "GET") 
			Get().response(client);
		else if (str == "DELETE")
			Delete().response(client);
		else
			client.error(501);
	};

	void  Server::_init( void )
	{
		mime["html"]	=	"text/html";
		mime["htm"]		=	"text/html";
		mime["shtml"]	=	"text/html";
		mime["css"]		=	"text/css";
		mime["xml"]		=	"text/xml";
		mime["gif"]		=	"image/gif";
		mime["jpeg"]	=	"image/jpeg";
		mime["jpg"]		=	"image/jpeg";
		mime["js"]		=	"application/javascript";
		mime["atom"]	=	"application/atom+xml";
		mime["rss"]		=	"application/rss+xml";

		mime["mml"]		=	"text/mathml";
		mime["txt"]		=	"text/plain";
		mime["jad"]		=	"text/vnd.sun.j2me.app-descriptor";
		mime["wml"]		=	"text/vnd.wap.wml";
		mime["htc"]		=	"text/x-component";

		mime["png"]		=	"image/png";
		mime["tif"]		=	"image/tiff";
		mime["tiff"]	=	"image/tiff";
		mime["wbmp"]	=	"image/vnd.wap.wbmp";
		mime["ico"]		=	"image/x-icon";
		mime["jng"]		=	"image/x-jng";
		mime["bmp"]		=	"image/x-ms-bmp";
		mime["svg"]		=	"image/svg+xml";
		mime["svgz"]	=	"image/svg+xml";
		mime["webp"]	=	"image/webp";

		mime["woff"]	=	"application/font-woff";
		mime["jar"]		=	"application/java-archive";
		mime["war"]		=	"application/java-archive";
		mime["ear"]		=	"application/java-archive";
		mime["json"]	=	"application/json";
		mime["hqx"]		=	"application/mac-binhex40";
		mime["doc"]		=	"application/msword";
		mime["pdf"]		=	"application/pdf";
		mime["ps"]		=	"application/postscript";
		mime["eps"]		=	"application/postscript";
		mime["ai"]		=	"application/postscript";
		mime["rtf"]		=	"application/rtf";
		mime["m3u8"]	=	"application/vnd.apple.mpegurl";
		mime["xls"]		=	"application/vnd.ms-excel";
		mime["eot"]		=	"application/vnd.ms-fontobject";
		mime["ppt"]		=	"application/vnd.ms-powerpoint";
		mime["wmlc"]	=	"application/vnd.wap.wmlc";
		mime["kml"]		=	"application/vnd.google-earth.kml+xml";
		mime["kmz"]		=	"application/vnd.google-earth.kmz";
		mime["7z"]		=	"application/x-7z-compressed";
		mime["cco"]		=	"application/x-cocoa";
		mime["jardiff"]	=	"application/x-java-archive-diff";
		mime["jnlp"]	=	"application/x-java-jnlp-file";
		mime["run"]		=	"application/x-makeself";
		mime["pl"]		=	"application/x-perl";
		mime["pm"]		=	"application/x-perl";
		mime["prc"]		=	"application/x-pilot";
		mime["pdb"]		=	"application/x-pilot";
		mime["rar"]		=	"application/x-rar-compressed";
		mime["rpm"]		=	"application/x-redhat-package-manager";
		mime["sea"]		=	"application/x-sea";
		mime["swf"]		=	"application/x-shockwave-flash";
		mime["sit"]		=	"application/x-stuffit";
		mime["tcl"]		=	"application/x-tcl";
		mime["tk"]		=	"application/x-tcl";
		mime["der"]		=	"application/x-x509-ca-cert";
		mime["pem"]		=	"application/x-x509-ca-cert";
		mime["crt"]		=	"application/x-x509-ca-cert";
		mime["xpi"]		=	"application/x-xpinstall";
		mime["xhtml"]	=	"application/xhtml+xml";
		mime["xspf"]	=	"application/xspf+xml";
		mime["zip"]		=	"application/zip";

		mime["bin"]		=	"application/octet-stream";
		mime["exe"]		=	"application/octet-stream";
		mime["dll"]		=	"application/octet-stream";
		mime["deb"]		=	"application/octet-stream";
		mime["dmg"]		=	"application/octet-stream";
		mime["iso"]		=	"application/octet-stream";
		mime["img"]		=	"application/octet-stream";
		mime["msi"]		=	"application/octet-stream";
		mime["msp"]		=	"application/octet-stream";
		mime["msm"]		=	"application/octet-stream";

		mime["docx"]	=	\
		 "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
		mime["xlsx"]	=	\
		 "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
		mime["pptx"]	=	\
		 "application/vnd.openxmlformats-officedocument.presentationml.presentation";

		mime["mid"]		=	"audio/midi";
		mime["midi"]	=	"audio/midi";
		mime["kar"]		=	"audio/midi";
		mime["mp3"]		=	"audio/mpeg";
		mime["ogg"]		=	"audio/ogg";
		mime["m4a"]		=	"audio/x-m4a";
		mime["ra"]		=	"audio/x-realaudio";

		mime["3gpp"]	=	"video/3gpp";
		mime["3gp"]		=	"video/3gpp";
		mime["ts"]		=	"video/mp2t";
		mime["mp4"]		=	"video/mp4";
		mime["mpeg"]	=	"video/mpeg";
		mime["mpg"]		=	"video/mpeg";
		mime["mov"]		=	"video/quicktime";
		mime["webm"]	=	"video/webm";
		mime["flv"]		=	"video/x-flv";
		mime["m4v"]		=	"video/x-m4v";
		mime["mng"]		=	"video/x-mng";
		mime["asx"]		=	"video/x-ms-asf";
		mime["asf"]		=	"video/x-ms-asf";
		mime["wmv"]		=	"video/x-ms-wmv";
		mime["avi"]		=	"video/x-msvideo";

		//Creation of default error pages map
		error[400] = "Bad Request";
		error[403] = "Forbidden";
		error[404] = "Not Found";
		error[405] = "Not Allowed";
		error[406] = "Not Acceptable";
		error[408] = "Request Timeout";
		error[411] = "Length Required";
		error[413] = "Content Too Large";
		error[414] = "URI Too Long";
		error[415] = "Unsuported Media Type";
		error[500] = "Internal Server Error";
		error[501] = "Not Implemented";
	}
}
