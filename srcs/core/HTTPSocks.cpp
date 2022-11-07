#include "HTTPSocks.hpp"

namespace ft
{
	HTTPSocks::~HTTPSocks( void )
	{
		this->clear();
	}

	HTTPSocks::HTTPSocks( void )
	{}

	HTTPSocks::HTTPSocks( HTTPSocks const& other )
	{
		*this = other;
	}

	HTTPSocks &
	HTTPSocks::operator=( HTTPSocks const& rhs )
	{
		this->clear();
		this->list = rhs.list;
		return *this;
	}

	t_sock_info *
	HTTPSocks::insert( uint16_t port )
	{
		t_sock_info	tmp;
		int			addrlen = sizeof(tmp.addr);
		int			enable = 1;

		memset(&tmp, 0, sizeof(tmp));
		tmp.fd = socket(AF_INET, SOCK_STREAM, 0);
		if (tmp.fd == -1)
			return (NULL);
		tmp.addr.sin_family = AF_INET;
		tmp.addr.sin_port = htons(port);
		tmp.addr.sin_addr.s_addr = htonl(INADDR_ANY);
		if (setsockopt(tmp.fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1)
		{
			close(tmp.fd);
			return (NULL);
		}
		if (bind(tmp.fd, (struct sockaddr *)&tmp.addr, addrlen) == -1)
		{
			close(tmp.fd);
			return (NULL);
		}
		if (listen(tmp.fd, S_MAX_CLIENT) == -1)
		{
			close(tmp.fd);
			return (NULL);
		}
		tmp.port = port;
		return &(*list.insert(list.begin(), tmp));
	}

	t_sock_info const*
	HTTPSocks::find( int sock_fd ) const
	{
		for (std::list<t_sock_info>::const_iterator it = list.begin();
			it != list.end(); it++)
		{
			if (it->fd == sock_fd)
				return &(*it);
		}
		return NULL;
	}

	void
	HTTPSocks::clear( void )
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
