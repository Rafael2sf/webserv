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

	static int stoh(char const* str, int * host)
	{
		int			byte = 0;
		char * 		ptr = (char *)str;

		*host = 0;
		for (int i = 0; i < 4; i++)
		{
			if (sscanf(ptr, "%d", &byte) <= 0)
				return -1;
			if (byte < 0 || byte > 255)
				return -1;
			*host += byte;
			if (i != 3)
				*host <<= 8;
			if (i != 3 && !(ptr = (char *)strchr(ptr, '.')))
				return -1;
			ptr += 1;
		}
		return 0;
	}

	static int stohp(char const* str, int * host, int * port)
	{
		char 		* p = (char *)strchr(str, ':');

		if (!p)
		{
			p =  (char *)strchr(str, '.');
			if (!p)
			{
				p = (char *)str;
				while (*p)
				{
					if (!isdigit(*p++))
						return -1;
				}
				if (sscanf(str, "%d", port) <= 0)
					return -1;
				*host = 0;
				return 0;
			}
			*port = 80;
			return stoh(str, host);
		}
		*port = 0;
		if (sscanf(p + 1, "%d", port) <= 0)
			return -1;
		p = (char *)strchr(str, 'l');
		if (p && !strncmp(p, "localhost", 9))
		{
			*host = 0x7f000001;
			return 0;
		}
		return stoh(str, host);
	}

	static int insertInitSock( t_sock_info & sock_info )
	{
		int			addrlen = sizeof(sock_info.addr);
		int			enable = 1;

		sock_info.fd = socket(AF_INET, SOCK_STREAM, 0);
		if (sock_info.fd == -1)
			return (-1);
		sock_info.addr.sin_family = AF_INET;
		sock_info.addr.sin_port = htons(sock_info.port);
		sock_info.addr.sin_addr.s_addr = htonl(sock_info.host);
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

	t_sock_info const*
	Sockets::insert( JSON::JsonToken * block )
	{
		JSON::JsonToken * port_token;
		t_sock_info		tmp;
		t_sock_info const*	match;

		memset(&tmp, 0, sizeof(tmp));
		port_token = block->find_first("listen");
		if (!port_token)
			return 0;
		if (stohp(port_token->as<char const*>(), &tmp.host, &tmp.port) == -1)
			return 0;
		tmp.conf = block;
		match = this->findByPort(tmp.port);
		if (match)
		{
			if (match->conf == block)
			{
				HTTP::err(1, "duplicate port in same block");
				return 0;
			}
			return match;
		}
		if (insertInitSock(tmp) != -1)
		{
			DEBUG2("listening .. " << tmp.port);
			return &(*list.insert(list.begin(), tmp));
		}
		return 0;
	}

	t_sock_info *
	Sockets::findByFd( int sock_fd )
	{
		for (std::list<t_sock_info>::iterator it = list.begin();
			it != list.end(); it++)
		{
			if (it->fd == sock_fd || it->clients.count(sock_fd))
				return &(*it);
		}
		return NULL;
	}

	t_sock_info const*
	Sockets::findByPort( int port ) const
	{
		for (std::list<t_sock_info>::const_iterator it = list.begin();
			it != list.end(); it++)
		{
			if (it->port == port)
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
