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

	int
	Server::init( char const* filepath )
	{
		if (conf.parse(filepath) < 0)
				return -1;
		DEBUG2("creating server sockets");
		for (JSON::Json::iterator it = conf.tokens.begin();
				it != conf.tokens.end(); it++ )
		{
			if (!socks.insert(*it))
			{
				HTTP::err(1);
				socks.clear();
				return -1;
			}
		}
		epoll.init(socks);
		return (0);
	}

	void
	Server::loop( void )
	{
		Mediator	med;
		t_sock_info *csock;
		
		if (socks.list.empty())
			exit(err(1, "logic error", "no sockets available"));
		while (1)
		{
			if (!state)
				break ;
			connectionTimer();
			int ev_count = epoll.wait();
			if (ev_count <= 0)
				continue;
			int ev_socket;
			for (int i = 0; i < ev_count; i++)
			{
				ev_socket = epoll.events[i].data.fd;
				csock = socks.findByFd(ev_socket);
				if (csock->fd == ev_socket)
				{
					if((ev_socket = accept(ev_socket, NULL, 0)) == -1)
						err(-1, "accept()");
					if (epoll.insert(ev_socket) == -1)
						err(-1, "insert()");
					csock->clients.insert(std::make_pair(ev_socket, time(NULL)));
				}
				else {
					try
					{
						csock->clients.at(ev_socket) = time(NULL);
						clientHandler(csock, i, med);
					}
					catch(const std::exception& e)
					{
						DEBUG2(e.what() << ev_socket);
					}

				}
			}
		}
	}

	/* this is to find the respective server block based on url, ik its a mess /
		FIX maybe:
			any location does not match /
			non dir location is matching /
			matchin wtihout / : ex /www matches /www/
	*/
	static JSON::JsonToken * findConfigOf(JSON::Json const& jc,
		t_sock_info const& csock, std::string const& path)
	{
		JSON::JsonObject * tmp;
		size_t		last_len = 0;
		JSON::JsonToken * last_match = 0;
		size_t		i = 0;

		for (JSON::Json::const_iterator it = jc.tokens.begin();
			it != jc.tokens.end(); it++)
		{
			try
			{
				if (csock.port == 8000)
				{
					tmp = dynamic_cast<JSON::JsonObject *>(&(*(*it))["location"]);
					if (!tmp)
						continue ;
					// loop trought all locations
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
				}
			}
			catch (std::exception const&) {}
			i = 0;
		}
		return last_match;
	}

	void	Server::clientHandler(t_sock_info * csock, int i, Mediator & med) {

		static char buffer[RECEIVE_BUF_SIZE] = {0};
		std::string	str;
		std::string	final_str;
		Message		request;
		
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

		request.init(final_str);
		if (!request.headers.empty())
			request.conf = findConfigOf(conf, *csock, request.getMethod()[1]);
		else
		{
			if (epoll.erase(epoll.events[i].data.fd) == -1)
				DEBUG2("epoll.erase() failed");
			csock->clients.erase(epoll.events[i].data.fd);
			return ;
		}

		if (request.conf)
		{
			std::cerr << std::endl;
			DEBUG2(request.getMethod()[0] << " " << request.getMethod()[1]);
			DEBUG2("location = " << request.conf->getProperty());
		}
		med.methodChoice(request, epoll.events[i].data.fd);
		if (request.getHeaderVal("connection") == "close")
		{
			if (epoll.erase(epoll.events[i].data.fd) == -1)
				DEBUG2("epoll.erase() failed");
			csock->clients.erase(epoll.events[i].data.fd);
		}
	}

	void	Server::connectionTimer(void) {
		
		double seconds = time(NULL);

		for (std::list<t_sock_info>::iterator sit = socks.list.begin();
			sit != socks.list.end(); sit++)
		{
			for (std::map<int, double>::iterator it = (*sit).clients.begin();
				it != (*sit).clients.end(); it++)
			{
				if (seconds - it->second >= 10) {
					DEBUG2(it->first << " was erased by timeout!!!!!");
					if (epoll.erase(it->first) == -1)
						DEBUG2("epoll.erase() failed");
					(*sit).clients.erase(it->first);
					it = (*sit).clients.begin();
					if (it == (*sit).clients.end())
						break;
				}
			}
		}
	}
}
