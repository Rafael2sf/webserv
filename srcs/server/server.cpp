/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: daalmeid <daalmeid@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/10/25 12:54:31 by daalmeid          #+#    #+#             */
/*   Updated: 2022/10/28 18:23:13 by daalmeid         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstdlib>		// exit() and EXIT_FAILURE
#include <iostream>		// For cout
#include <cstring>		// memset()
#include <cerrno>		// errno

extern "C"
{
	#include <sys/socket.h> // Socket functions
	#include <netinet/in.h> // sockaddr_in
	#include <unistd.h>		// read
	#include <arpa/inet.h>	// htonl, htons, ntohl, ntohs
	#include <sys/epoll.h>	// for epoll_create1(), epoll_ctl(), struct epoll_event
	#include <fcntl.h>		// fctnl()
}

#define MAX_CLIENT 10
#define PORT1 8080
#define PORT2 9090
#define EPOLL_SIZE 5
#define EPOLL_TIMEOUT -1

#define FT_ERROR(A, B) std::cerr << A << ": " << B << strerror(errno) << std::endl
#define DEBUG(X) std::cout << X << std::endl

typedef struct	s_httpsock
{
	sockaddr_in		addr;
	int				sock;
}	t_httpsock;

typedef struct s_sockpoll
{
	struct epoll_event	events[EPOLL_SIZE];
	int					fd;
	int					size;
} t_sockpoll;

int ft_setsockflags(int __fd, int __flags)
{
	if (__flags == -1)
		__flags = 0;
	return fcntl(__fd, F_SETFL, __flags);
}

int ft_serversock(int __port, sockaddr_in &__addr, int __max)
{
	int		sock;
	int		enable = 1;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
		FT_ERROR("socket()", 1);
	memset(&__addr, 0, sizeof(__addr));
	__addr.sin_family = AF_INET;
	__addr.sin_port = htons(__port);
	__addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1)
		FT_ERROR("setsockopt()", 1);
	if (bind(sock, (struct sockaddr *)&__addr, sizeof(__addr)) == -1)
		FT_ERROR("bind()", 1);
	if (listen(sock, __max) == -1)
		FT_ERROR("listen()", 1);
	return (sock);
}

int ft_sockpolladd(int __epfd, int __sofd)
{
	epoll_event		ev;

	ev.events = EPOLLIN;
	ev.data.fd = __sofd;
	return (epoll_ctl(__epfd, EPOLL_CTL_ADD, __sofd, &ev));
}

void ft_handle(t_sockpoll & server_epoll, int i)
{
	/*
		HTTPRes
		build the http response
	*/
	static char buffer[30000] = {0};
	const char *hello = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";

	int valread = recv(server_epoll.events[i].data.fd, buffer, 30000, 0);
	if (valread == -1)
		DEBUG("client disconnect");
	DEBUG(buffer);
	send(server_epoll.events[i].data.fd, hello, strlen(hello), 0);
	DEBUG("message sent");
	if (epoll_ctl(server_epoll.fd, EPOLL_CTL_DEL, server_epoll.events[i].data.fd, NULL) == -1)
		FT_ERROR("epoll_ctl()", 1);
	close(server_epoll.events[i].data.fd);
}

int main(void)
{
	/*
		class HTTPSocks
			.insert (PORT)
			.erase (PORT)
			[ ... ]
		store and manipulate all server listening sockets
	*/
	t_httpsock	server_sock1, server_sock2;
	server_sock1.sock = ft_serversock(PORT1, server_sock1.addr, MAX_CLIENT);
	server_sock2.sock = ft_serversock(PORT2, server_sock2.addr, MAX_CLIENT);
	DEBUG("server sockets initialized");

	/*
		class HTTPoll
			.insert (SOCK)
			.erase (SOCK)
			.wait ()
			[ ... ]
		a wrapper class to epoll
	*/
	t_sockpoll	server_epoll;
	server_epoll.size = EPOLL_SIZE;
	server_epoll.fd = epoll_create(EPOLL_SIZE);
	if (server_epoll.fd  == -1)
		FT_ERROR("epoll_create()", -1);
	if (ft_sockpolladd(server_epoll.fd, server_sock1.sock) == -1)
		FT_ERROR("ft_sockpolladd()", -1);
	if (ft_sockpolladd(server_epoll.fd, server_sock2.sock) == -1)
		FT_ERROR("ft_sockpolladd()", -1);
	DEBUG("server ready to listen");

	int		ev_count = -1;
	int		ev_socket = -1;

	while (1)
	{
		DEBUG("\nlistening ..\n");
		ev_count = epoll_wait(server_epoll.fd, server_epoll.events,
			 EPOLL_SIZE, EPOLL_TIMEOUT);
		if (ev_count == -1)
			FT_ERROR("epoll_wait()", -1);

		DEBUG("[" << ev_count << "] ready events");
		for (int i = 0; i < ev_count; i++)
		{
			ev_socket = server_epoll.events[i].data.fd;
			if (server_epoll.events[i].data.fd == server_sock1.sock
				|| server_epoll.events[i].data.fd == server_sock2.sock)
			{
				ev_socket = accept(ev_socket, NULL, 0);
				if (ev_socket == -1)
					FT_ERROR("accept()", -1);
				if (ft_setsockflags(ev_socket,
					fcntl(ev_socket, F_GETFL, 0) | O_NONBLOCK) == -1)
					FT_ERROR("ft_setsockflags()", -1);
				if (ft_sockpolladd(server_epoll.fd, ev_socket) == -1)
					FT_ERROR("ft_sockpolladd()", -1);
				/*
					HTTPReq
					parse the raw text received into a struct/class
				*/
			}
			else
				ft_handle(server_epoll, i);
		}
	}
	return 0;
}
