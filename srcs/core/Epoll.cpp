/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Epoll.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: daalmeid <daalmeid@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/10/31 14:31:27 by daalmeid          #+#    #+#             */
/*   Updated: 2022/11/23 18:33:50 by daalmeid         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Epoll.hpp"
#include "Sockets.hpp"

namespace HTTP {

	Epoll::Epoll(void)
	: fd(-1)
	{};

	Epoll::Epoll(Epoll const& cpy)
	: fd(-1)
	{
		*this = cpy;
	};

	Epoll & Epoll::operator=(Epoll const& cpy)
	{
		(void) cpy;
		return *this;
	};

	epoll_event & Epoll::operator[](size_t index)
	{
		return events[index];
	};

	Epoll::~Epoll(void)
	{};

	int	Epoll::init(Sockets const& socks)
	{
		this->size = S_MAX_CLIENT;
		this->fd = epoll_create(S_MAX_CLIENT);
		if (this->fd  == -1)
			return err(-1);
		for (std::list<t_sock_info>::const_iterator it = socks.list.begin();
			it != socks.list.end(); it++)
		{
			if (it->fd != -1 && insert(it->fd, LISTEN_SOCKET) == -1)
				return err(-1);
		}
		return 0;
	};

	int	Epoll::insert(int sofd, int flag)
	{
		epoll_event		ev;

		memset(&ev, 0, sizeof(ev));
		ev.events = EPOLLIN | EPOLLOUT;
		if (flag == LISTEN_SOCKET)
			ev.events = EPOLLIN;
		ev.data.fd = sofd;
		return (epoll_ctl(this->fd, EPOLL_CTL_ADD, sofd, &ev));
	}

	int	Epoll::erase(int cli_fd)
	{
		if (epoll_ctl(this->fd, EPOLL_CTL_DEL,
				cli_fd, NULL) == -1)
			return -1;
		close(cli_fd);
		return 0;
	};

	int	Epoll::wait(void)
	{
		return epoll_wait(this->fd, this->events,
			S_MAX_CLIENT, S_EPOLL_TIMEOUT);
	}

	void Epoll::stop( void )
	{
		if (fd >= 0)
		{
			close(fd);
			fd = -1;
		}
	}
}
