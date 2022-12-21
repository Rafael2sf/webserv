#include "Sockets.hpp"
#include "Server.hpp"

namespace HTTP
{
	Sockets::~Sockets( void )
	{
		this->clear();
	}

	Sockets::Sockets( void )
	{}

	Sockets::Sockets( Sockets const& other )
	{
		*this = other;
	}

	Sockets &
	Sockets::operator=( Sockets const& rhs )
	{
		this->clear();
		this->list = rhs.list;
		return *this;
	}

	static int isock( t_sock_info & sock_info )
	{
		int			addrlen = sizeof(sock_info.addr);
		int			enable = 1;

		sock_info.fd = socket(AF_INET, SOCK_STREAM, 0);
		if (sock_info.fd == -1)
			return (-1);
		// sock_info.addr.sin_family = AF_INET;
		// sock_info.addr.sin_port = htons(sock_info.port);
		// sock_info.addr.sin_addr.s_addr = htonl(sock_info.host);
		if (setsockopt(sock_info.fd, SOL_SOCKET, SO_REUSEADDR, 
			&enable, sizeof(int)) == -1)
		{
			close(sock_info.fd);
			return (-1);
		}
		if (bind(sock_info.fd, 
			(struct sockaddr *)&sock_info.addr, addrlen) == -1)
		{
			close(sock_info.fd);
			return (-1);
		}
		if (listen(sock_info.fd, S_MAX_CLIENT) == -1)
		{
			close(sock_info.fd);
			return (-1);
		}
		return (0);
	}

	t_sock_info *
	Sockets::insert( u_int32_t host, u_int16_t port )
	{
		t_sock_info si;

		memset(&si, 0, sizeof(si));
		si.fd = -1;
		si.addr.sin_family = AF_INET;
		si.addr.sin_port = htons(port);
		si.addr.sin_addr.s_addr = htonl(host);
		return &(*list.insert(list.end(), si));
	}

	static t_sock_info const* usedHostPort(
	 std::vector<t_sock_info*>::const_iterator begin,
	 std::vector<t_sock_info*>::const_iterator end,
	 uint32_t host, uint16_t port)
	{
		while (begin != end)
		{
			if (((*begin)->addr.sin_addr.s_addr == host
				|| (*begin)->addr.sin_addr.s_addr == 0)
				&& (*begin)->addr.sin_port == port)
			{
				return *begin;
			}
			begin++;
		}
		return 0;
	}

	bool hasAnyHost(
	 std::list<t_sock_info>::const_iterator begin,
	 std::list<t_sock_info>::const_iterator end,
	 uint16_t port)
	{
		while (begin != end)
		{
			if (begin->addr.sin_addr.s_addr == 0
				&& begin->addr.sin_port == port)
			{
				return true;
			}
			begin++;
		}
		return false;
	}

	// -> search for used port
	// -> if not used search for any -> if foound 

	int Sockets::listen( void )
	{
		std::vector<t_sock_info*> used;
		unsigned int port;

		DEBUG(std::cerr << "\n| Sockets |\n");
		for (std::list<t_sock_info>::iterator it = list.begin();
			it != list.end(); it++)
		{
			port = ntohl((*it).addr.sin_addr.s_addr);
			DEBUG(std::cerr << "[socket] " \
				<< ((port & 0xff000000) >> 24) << '.' \
				<< ((port & 0x00ff0000) >> 16) << '.' \
				<< ((port & 0x0000ff00) >> 8) << '.' \
				<< (port & 0x000000ff) << ':' << htons((*it).addr.sin_port));
			if (usedHostPort(used.begin(), used.end(),
				it->addr.sin_addr.s_addr, it->addr.sin_port))
			{
				DEBUG(std::cerr << " [inactive]" << std::endl;);
				continue ;
			}
			if (it->addr.sin_addr.s_addr != 0
			&& hasAnyHost(list.begin(), list.end(), it->addr.sin_port))
			{
				DEBUG(std::cerr << " [inactive]" << std::endl;);
				continue ;
			}
			if (isock(*it) == -1)
			{
				std::cerr << "webserv: " \
					<< ((port & 0xff000000) >> 24) << '.' \
					<< ((port & 0x00ff0000) >> 16) << '.' \
					<< ((port & 0x0000ff00) >> 8) << '.' \
					<< (port & 0x000000ff) << ':'
					<< htons((*it).addr.sin_port) << ": ";
				std::perror("");
				while (it != list.begin())
				{
					if ((*it).fd != 1)
						close((*it).fd);
					it--;
				}
				if ((*it).fd != -1)
					close((*it).fd);
				return -1;
			}
			DEBUG(std::cerr << " [active]" << std::endl;);
			used.push_back(&*it);
		}
		return 0;
	}

	t_sock_info *
	Sockets::find( int sock_fd )
	{
		for (std::list<t_sock_info>::iterator it = list.begin();
			it != list.end(); it++)
		{
			if (it->fd == sock_fd && it->fd != -1)// || it->clients.count(sock_fd))
				return &(*it);
		}
		return NULL;
	}

	void
	Sockets::clear( void )
	{
		if (list.size() > 0)
		{
			for (std::list<t_sock_info>::iterator it = list.begin();
				it != list.end(); it++)
			{
				close(it->fd);
			}
			list.clear();
		}
	}
}
