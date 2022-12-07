#include "Server.hpp"

namespace HTTP
{
	int Server::state = 1;

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
		DEBUG2("called non-implemented function: Server::operator=( Server const& rhs )");
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
					so.insert(host, port)->config = &*it;
					used.push_back(std::make_pair(host, port));
				}
			}
		}
		return 0;
	}

	int
	Server::init(char const *filepath)
	{
		DEBUG2("reading configuration file");
		if (config.from(filepath) < 0)
			return -1;
		DEBUG2("mapping server sockets");
		if (listenMap(config, socks) == -1)
			return -1;
		DEBUG2("initiating server sockets");
		if (socks.listen() == -1)
			return -1;
		epoll.init(socks);
		DEBUG2("ready");
		return (0);
	}

	void
	Server::_receiveConnection( int socket, t_sock_info * si )
	{
		Client * 			cl;
		struct sockaddr_in	addr;
		socklen_t			addrlen;

		memset(&addr, 0, sizeof(addr));
		try
		{
			if ((socket = accept(
				socket, (struct sockaddr *)&addr, &addrlen)) == -1)
			{
				err(-1);
				return ;
			}
			if (epoll.insert(socket) == -1)
			{
				err(-1);
				return ;
			}
			cl = &clients.insert(clients.end(), 
				std::make_pair(socket, Client()))->second;
			cl->fd = socket;
			cl->ai.sin_addr.s_addr = addr.sin_addr.s_addr;
			cl->ai.sin_port = si->addr.sin_port;
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
	Server::_updateConnection( int i, int socket, Mediator & med )
	{
		Client * cl;

		(void) med;
		try
		{
			cl = &clients.at(socket);
			//cl->timestamp = time(NULL);
			if (cl->update() == -1)
			{
				write(cl->fd, "[408] failed parsing", 3);
				if (epoll.erase(epoll.events[i].data.fd) == -1)
					DEBUG2("epoll.erase() failed");
				clients.erase(epoll.events[i].data.fd);
			}
			if (cl->ok())
				ft_handle(*cl, i, med);
		}
		catch (const std::exception &e)
		{
			DEBUG2("[500] failed to update client: " << e.what());
			clients.erase(socket);
			epoll.erase(socket);
			close(socket);
		}
	}

	void
	Server::loop(void)
	{
		t_sock_info *	si;
		Mediator 		med;
		int				socket;
		int				events;

		// if (socks.list.empty())
		// 	exit(err(1, "logic error", "no sockets available"));
		while (1)
		{
			if (!state)
				break;
			check_times();
			events = epoll.wait();
			if (events <= 0)
				continue;
			for (int i = 0; i < events; i++)
			{
				socket = epoll.events[i].data.fd;
				si = socks.find(socket);
				if (si)
					_receiveConnection(socket, si);
				else
					_updateConnection(i, socket, med);
			}
		}
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
			return 0;
		else if (interest.size() == 1)
			return (*interest.begin())->config;
		// TODO -- look at server_name
		return (*interest.begin())->config;
	}

	static JSON::Node *matchLocation(JSON::Node *serv, std::string const &path)
	{
		JSON::Object *tmp;
		size_t last_len = 0;
		JSON::Node *last_match = 0;
		size_t i = 0;

		tmp = dynamic_cast<JSON::Object *>(serv->search(1, "location"));
		if (!tmp)
			return 0;
		for (JSON::Node::iterator loc = tmp->begin(); loc != tmp->end(); loc++)
		{
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
		return last_match;
	}

	void Server::ft_handle(Client &cli, int i, Mediator &med)
	{

		// static char buffer[RECEIVE_BUF_SIZE] = {0};
		// std::string str;
		// std::string final_str;

		// int valread = recv(epoll.events[i].data.fd, buffer, RECEIVE_BUF_SIZE, 0);
		// if (valread == -1)
		// 	DEBUG2("client disconnect");
		// if (valread == 0)
		// {
		// 	if (epoll.erase(epoll.events[i].data.fd) == -1)
		// 		DEBUG2("epoll.erase() failed");
		// 	clients.erase(epoll.events[i].data.fd);
		// 	return ;
		// }
		// while (valread > 0)
		// {
		// 	str.assign(buffer, valread);
		// 	final_str += str;
		// 	usleep(500);
		// 	valread = recv(epoll.events[i].data.fd, buffer, RECEIVE_BUF_SIZE, 0);
		// 	if (valread == -1)
		// 		perror("recv error: ");
		// }

		//cli.req.init(final_str);
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
			return;
		}

		if (cli.req.conf)
		{
			unsigned int port = htonl(cli.ai.sin_addr.s_addr);
			std::cerr << std::endl;
			DEBUG2('[' << epoll.events[i].data.fd << "] [FROM " \
				<< ((port & 0xff000000) >> 24) << '.' \
				<< ((port & 0x00ff0000) >> 16) << '.' \
				<< ((port & 0x0000ff00) >> 8) << '.' \
				<< (port & 0x000000ff) << ':' << htons(cli.ai.sin_port) << "] [" \
				<< cli.req.get_method()[0] << ' ' << cli.req.get_method()[1] \
				<< "] [location " << cli.req.conf->getProperty() << ']');
		}
		//DEBUG2(cli.req.response_string());
		med.method_choice(cli.req, epoll.events[i].data.fd);
		if (cli.req.get_head_val("connection") == "close")
		{
			if (epoll.erase(epoll.events[i].data.fd) == -1)
				DEBUG2("epoll.erase() failed");
			clients.erase(epoll.events[i].data.fd);
		}
		cli.reset();
	}

	void Server::check_times(void)
	{

		double seconds = time(NULL);

		for (std::map<int, Client>::iterator it = clients.begin();
			 it != clients.end(); it++)
		{
			if (seconds - it->second.timestamp >= 10)
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
}
