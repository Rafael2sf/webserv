#include "HTTPServer.hpp"

namespace ft
{
	int HTTPServer::state = 1;


	HTTPServer::~HTTPServer( void )
	{}

	HTTPServer::HTTPServer( void )
	{}

	HTTPServer::HTTPServer( HTTPServer const& other )
	{
		*this = other;
	}

	HTTPServer &
	HTTPServer::operator=( HTTPServer const& rhs )
	{
		DEBUG2("called non-implemented function: HTTPServer::operator=( HTTPServer const& rhs )");
		(void) rhs;
		return *this;
	}

	int
	HTTPServer::init( void )
	{
		DEBUG2("server default init");
		return 0;
	}

	int
	HTTPServer::init( char const* filepath )
	{
		if (conf.parse(filepath) < 0)
				return -1;
		DEBUG2("creating server sockets");
		for (ft::Json::iterator it = conf.tokens.begin();
				it != conf.tokens.end(); it++ )
		{
			if (!socks.insert(*it))
			{
				ft::err(1);
				socks.clear();
				return -1;
			}
		}
		epoll.init(socks);
		return (0);
	}

	void
	HTTPServer::loop( void )
	{
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
			//DEBUG2("[" << ev_count << "] ready events");
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
						ft_handle(csock, i, med);
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
	static JsonToken * findConfigOf(Json const& jc,
		t_sock_info const& csock, std::string const& path)
	{
		
		for (Json::const_iterator it = jc.tokens.begin();
			it != jc.tokens.end(); it++)
		{
			try {
				if (csock.port == ((*(*it))["listen"]).as<int>())
					return &((*(*it))["location"][path.c_str()]);
			}
			catch (std::exception const&) {}
		}

		JsonObject * tmp;
		size_t		last_len = 0;
		size_t		i = 0;
		JsonToken * last_match = 0;

		for (Json::const_iterator it = jc.tokens.begin();
			it != jc.tokens.end(); it++)
		{
			try
			{
				if (csock.port == ((*(*it))["listen"]).as<int>())
				{
					tmp = static_cast<JsonObject *>(&(*(*it))["location"]);
					// loop trought all locations
					for (std::vector<JsonToken*>::iterator loc = tmp->data.begin();
						loc != tmp->data.end(); loc++)
					{
						// if url matches a block
						while (i < strlen((*loc)->getProperty())
							&& (*loc)->getProperty()[i] == path[i])
							i++;
						// find dir
						while (i != __SIZE_MAX__ && (*loc)->getProperty()[i] != '/')
							i--;
						/*
							if the whole location is contained in url and its bigger then
							previous match, update
						*/
						if (i == strlen((*loc)->getProperty()) - 1
							&& i > last_len)
						{
							last_len = i;
							last_match = *loc;
						}
					}
				}
			}
			catch (std::exception const&) {}
			i = 0;
		}
		/* TODO
			if no match:
				search (pre wildcard) *.
				search (post wildcard) .*
		*/
		return last_match;
	}

	void	HTTPServer::ft_handle(t_sock_info * csock, int i, Mediator & med) {

		static char buffer[30000] = {0};

		int valread = recv(epoll.events[i].data.fd, buffer, 30000, 0);
		if (valread == -1)
			DEBUG2("client disconnect");

		HTTPReq	request(buffer);
		if (!request.headers.empty())
			request.conf = findConfigOf(conf, *csock, request.get_method()[1]);
		if (request.conf)
		{
			std::cerr << std::endl;
			DEBUG2(request.get_method()[0] << " " << request.get_method()[1]);
			DEBUG2("location = " << request.conf->getProperty());
		}
		med.method_choice(request, epoll.events[i].data.fd);

		//DEBUG2("message sent");
		if (request.get_head_val("Connection") == "close"
				|| valread == 0)
		{
			//DEBUG2(epoll.events[i].data.fd << "erased");
			if (epoll.erase(epoll.events[i].data.fd) == -1)
				DEBUG2("epoll.erase() failed");
			csock->clients.erase(epoll.events[i].data.fd);
		}
	}

	void	HTTPServer::check_times(void) {
		
		double seconds = time(NULL);

		for (std::list<t_sock_info>::iterator sit = socks.list.begin();
			sit != socks.list.end(); sit++)
		{
			for (std::map<int, double>::iterator it = (*sit).clients.begin();
				it != (*sit).clients.end(); it++)
			{
				if (seconds - it->second >= 10) {
					//DEBUG2(it->first << " erased by timeout");	
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
