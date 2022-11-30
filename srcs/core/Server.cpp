#include "Server.hpp"

namespace HTTP
{
	int Server::state = 1;


	Server::~Server( void )
	{}

	Server::Server( void )
	{}

	Server::Server( Server const& other )
	{
		*this = other;
	}

	Server &
	Server::operator=( Server const& rhs )
	{
		DEBUG2("called non-implemented function: Server::operator=( Server const& rhs )");
		(void) rhs;
		return *this;
	}

	int
	Server::init( void )
	{
		DEBUG2("server default init");
		return 0;
	}

	static int stohp_validate( char const* s )
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
		while (isdigit(s[i]))
			i++;
		if (s[i])
			return -1;
		return 0;
	}

	static int stohp( char const* s, unsigned int * host, unsigned int * port )
	{
		int tmp[4];

		*host = 0;
		*port = 0;
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
			memset(tmp, 0, sizeof(int) * 4);
			if (sscanf(s, "%d", port) != 1)
				return -1;
		}
		if (*port < 0 || *port > 65535)
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

	int listenMap(JSON::Json const& json, Sockets & so)
	{
		unsigned int host, port;
		JSON::JsonToken * t;
		std::vector<int> ports;

		for (JSON::Json::const_iterator it = json.tokens.begin();
				it != json.tokens.end(); it++ )
		{
			t = (*it)->search(1, "listen");
			if (!t)
			{
				host = 0;
				port = 8000;
			}
			else if (t->getType() == JSON::json_array_type)
			{
				for (std::vector<JSON::JsonToken *>::const_iterator arr \
					 = dynamic_cast<JSON::JsonArray*>(t)->data.begin();
					arr != dynamic_cast<JSON::JsonArray*>(t)->data.end(); arr++ )
				{
					if (stohp((*arr)->as<char const*>(), &host, &port) == -1)
						return err(-1, "invalid listen field");
					if (std::find(ports.begin(), ports.end(), port) != ports.end())
						return err(-1, "duplicate listen field");
					so.insert(host, port)->config = *it;
					ports.push_back(port);
				}
				ports.clear();
				continue ;
			}
			else
			{
				if (stohp(t->as<char const*>(), &host, &port) == -1)
					return err(-1, "invalid listen field");
			}
			so.insert(host, port)->config = *it;
		}
		return 0;
	}

	int
	Server::init( char const* filepath )
	{
		if (conf.parse(filepath) < 0)
			return -1;
		DEBUG2("creating server sockets");
		if (listenMap(conf, socks) == -1)
			return -1;
		if (socks.listen() == -1)
			return -1;
		// for ( std::list<t_sock_info>::iterator it = socks.list.begin();
		// 		it != socks.list.end(); it++ )
		// {
		// 	DEBUG2("^^ " << (*it).port);
		// 	DEBUG2("test: " << (*it).config->search(1, "test")->as<char const*>());
		// }
		epoll.init(socks);
		return (0);
	}

	// void test(t_sock_info * si)
	// {
	// 	struct addrinfo *res;
	// 	struct addrinfo *it;

	// 	(void) si;
	// 	if (getaddrinfo("", NULL, NULL, &res) != 0)
	// 		return ;
	// 	for (it = res; it != NULL; it = it->ai_next)
	// 	{
	// 		char hostname[NI_MAXHOST] = "";
	// 		if (getnameinfo(it->ai_addr, it->ai_addrlen, hostname, NI_MAXHOST, NULL, 0, 0) != 0)
	// 			continue ;
	// 		DEBUG2("hostname = " << hostname);
	// 	}
	// 	freeaddrinfo(res);
	// }

	void
	Server::loop( void )
	{
		Client * ci;
		Mediator	med;
		t_sock_info *csock;
		
		if (socks.list.empty())
			exit(err(1, "logic error", "no sockets available"));
		while (1)
		{
			if (!state)
				break ;
			check_times();
			int ev_count = epoll.wait();
			if (ev_count <= 0)
				continue;
			int ev_socket;
			for (int i = 0; i < ev_count; i++)
			{
				ev_socket = epoll.events[i].data.fd;
				csock = socks.find(ev_socket);
				if (csock)
				{
					if ((ev_socket = accept(ev_socket, 0, 0)) == -1)
						err(-1, "accept()");
					if (epoll.insert(ev_socket) == -1)
						err(-1, "insert()");

					ci = &clients.insert(
						clients.end(), std::make_pair(ev_socket, Client()))->second;
					ci->fd = ev_socket;
					ci->ai.sin_addr.s_addr = csock->addr.sin_addr.s_addr;
					ci->ai.sin_port = csock->addr.sin_port;
					ci->timestamp = time(NULL);
				}
				else
				{
					try
					{
						ci = &clients.at(ev_socket);
						ci->timestamp = time(NULL);
						ft_handle(*ci, i, med);
					}
					catch(const std::exception& e)
					{
						DEBUG2(e.what() << ev_socket);
					}
				}
			}
		}
	}

	static JSON::JsonToken * matchCon(Sockets & so, Client const& cli)
	{
		std::list<t_sock_info *> interest;
		bool perfect_match = 0;

		for (std::list<t_sock_info>::iterator it = so.list.begin();
			it != so.list.end(); it++)
		{
			if ((*it).addr.sin_port == cli.ai.sin_port
				&& (((*it).addr.sin_addr.s_addr == cli.ai.sin_addr.s_addr)
				|| cli.ai.sin_addr.s_addr == 0))
			{
				if (((*it).addr.sin_addr.s_addr) != 0)
					perfect_match = true;
				interest.push_back(&(*it));
			}
		}
		// if full match remove 0.0.0.0
		if (perfect_match && cli.ai.sin_addr.s_addr != 0)
		{
			for (std::list<t_sock_info *>::iterator it = interest.begin();
				it != interest.end(); it++)
			{
				if (((*it)->addr.sin_addr.s_addr) == 0)
				{
					it++;
					interest.erase(it);
				}

			}
		}
		if (interest.size() == 0)
		{
			DEBUG2("RIP");
			return 0;
		}
		else if (interest.size() == 1)
			return (*interest.begin())->config;
		// TODO -- look at server_name
		return (*interest.begin())->config;
	}

	static JSON::JsonToken * matchLocation(JSON::JsonToken * serv, std::string const& path)
	{
		JSON::JsonObject *	tmp;
		size_t				last_len = 0;
		JSON::JsonToken * 	last_match = 0;
		size_t				i = 0;

		tmp = dynamic_cast<JSON::JsonObject *>(&((*serv)["location"]));
		if (!tmp)
			return 0;
		for (std::vector<JSON::JsonToken*>::iterator loc = tmp->data.begin();
			loc != tmp->data.end(); loc++)
		{
			if (path.size() == 1)
				i = path.find_first_of(*path.c_str());
			else
				i = path.find((*loc)->getProperty());
			if (i != std::string::npos && i == 0)
			{
				i = strlen((*loc)->getProperty());
				if (path.size() == i)
					return (*loc);
				if (i > last_len)
				{
					last_len = i;
					last_match = *loc;
				}
			}
		}
		return last_match;
	}

	void	Server::ft_handle(Client & cli, int i, Mediator & med) {

		static char buffer[RECEIVE_BUF_SIZE] = {0};
		std::string	str;
		std::string	final_str;

		int	valread = recv(epoll.events[i].data.fd, buffer, RECEIVE_BUF_SIZE, 0);
		if (valread == -1)
			DEBUG2("client disconnect");
		while (valread > 0) {
			str.assign(buffer, valread);
			final_str += str;
			usleep(500);
			valread = recv(epoll.events[i].data.fd, buffer, RECEIVE_BUF_SIZE, 0);
			if (valread == -1)
				perror("recv error: ");
		}

		cli.req.init(final_str);
		if (!cli.req.headers.empty())
		{
			cli.req.conf = matchCon(socks, cli);
			if (cli.req.conf)
				cli.req.conf = matchLocation(cli.req.conf, cli.req.get_method()[1]);
		}
		else
		{
			if (epoll.erase(epoll.events[i].data.fd) == -1)
				DEBUG2("epoll.erase() failed");
			clients.erase(epoll.events[i].data.fd);
			return ;
		}

		if (cli.req.conf)
		{
			std::cerr << std::endl;
			DEBUG2(cli.req.get_method()[0] << " " << cli.req.get_method()[1]);
			DEBUG2("location = " << cli.req.conf->getProperty());
		}
		med.method_choice(cli.req, epoll.events[i].data.fd);
		if (cli.req.get_head_val("connection") == "close")
		{
			if (epoll.erase(epoll.events[i].data.fd) == -1)
				DEBUG2("epoll.erase() failed");
			clients.erase(epoll.events[i].data.fd);
		}
	}

	void	Server::check_times(void) {
		
		double seconds = time(NULL);

		for (std::map<int, Client>::iterator it = clients.begin();
			it != clients.end(); it++)
		{
			if (seconds - it->second.timestamp >= 10) {
				DEBUG2(it->first << " was erased by timeout!!!!!");
				if (epoll.erase(it->first) == -1)
					DEBUG2("epoll.erase() failed");
				clients.erase(it->first);
				it = clients.begin();
				if (it == clients.end())
					break;
			}
		}
	}
}
