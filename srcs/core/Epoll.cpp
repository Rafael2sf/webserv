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
	{};

	Epoll::Epoll(Epoll const& cpy)
	{
		*this = cpy;
	};

	Epoll & Epoll::operator=(Epoll const& cpy)
	{
		DEBUG2("DO NOT CALL THIS COPY OPERATOR");
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
			return err(-1, "error in epoll_create()");
		for (std::list<t_sock_info>::const_iterator it = socks.list.begin();
			it != socks.list.end(); it++)
		{
			if (it->fd != -1 && insert(it->fd, LISTEN_SOCKET) == -1)
				return err(-1, "error in insert()");
		}
		return 0;
	};

	int	Epoll::insert(int sofd, int flag)
	{
		int				flags;
		epoll_event		ev;

		memset(&ev, 0, sizeof(ev));
		ev.events = EPOLLIN | EPOLLOUT;
		if (flag == LISTEN_SOCKET)
			ev.events = EPOLLIN;
		ev.data.fd = sofd;
		flags = fcntl(sofd, F_GETFL, 0);
		if (flags == -1)
			return -1;
		if (fcntl(sofd, F_SETFL, flags | O_NONBLOCK )  == -1)
			return -1;
		return (epoll_ctl(this->fd, EPOLL_CTL_ADD, sofd, &ev));
	}

	int	Epoll::erase(int cli_fd)
	{
		if (epoll_ctl(this->fd, EPOLL_CTL_DEL,
				cli_fd, NULL) == -1)
			return -1;
		if (close(cli_fd) == -1)
			DEBUG2("IS ALREADY CLOSED!");
		return 0;
	};
	
	int	Epoll::wait(void)
	{
		return epoll_wait(this->fd, this->events,
			S_MAX_CLIENT, S_EPOLL_TIMEOUT);
	}
}
