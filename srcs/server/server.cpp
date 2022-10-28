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

#include <sys/socket.h> // Socket functions
#include <netinet/in.h> // sockaddr_in
#include <cstdlib> // exit() and EXIT_FAILURE
#include <iostream> // For cout
#include <unistd.h> // read
#include <arpa/inet.h> // htonl, htons, ntohl, ntohs
#include <cstring> // memset()
#include <sys/epoll.h> // for epoll_create1(), epoll_ctl(), struct epoll_event
#include <fcntl.h>

int setnonblocking(int fd)
{
    int flags;
    if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
        flags = 0;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int	main(void) {

	//socket 8080
	int	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock ==  -1) {
		std::cerr << "error creating socket" << std::endl;
		return 1;
	}

	sockaddr_in	addr;
	int			addrlen = sizeof(addr);

	memset((char *)&addr, 0, addrlen);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8080);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));

	//socket 9595
	int	other_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (other_sock ==  -1) {
		std::cerr << "error creating socket" << std::endl;
		return 1;
	}
	
	sockaddr_in	other_addr;
	int			other_addrlen = sizeof(other_addr);

	memset((char *)&other_addr, 0, other_addrlen);
	other_addr.sin_family = AF_INET;
	other_addr.sin_port = htons(9595);
	other_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	memset(other_addr.sin_zero, '\0', sizeof(other_addr.sin_zero));

	//setnonblocking(sock);
	const int enable = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1)
		std::cerr << "error setsockopt" << std::endl;

	if (bind(sock, (struct sockaddr *) &addr, addrlen) == -1) {
		std::cerr << "error binding socket" << std::endl;
		return 1;
	}
	if (listen(sock, 10) == -1) {
		std::cerr << "error in listen" << std::endl;
		return 1;
	}

	if (setsockopt(other_sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1)
		std::cerr << "error setsockopt" << std::endl;

	if (bind(other_sock, (struct sockaddr *) &other_addr, other_addrlen) == -1) {
		std::cerr << "error binding socket" << std::endl;
		return 1;
	}
	if (listen(other_sock, 10) == -1) {
		std::cerr << "error in listen" << std::endl;
		return 1;
	}
	
	int epoll_fd = epoll_create(5);
	if (epoll_fd == -1) {
		std::cerr << "error in epoll_create1()" << std::endl;
		return 1;
	}
	
	
	struct epoll_event event;
	struct epoll_event events[5];
	int	event_count;
	
	event.events = EPOLLIN;
	event.data.fd = sock;

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock, &event)) {
		std::cerr << "Error in epoll_ctl()" << std::endl;
		close(epoll_fd);
		return 1;
	}

	event.events = EPOLLIN;
	event.data.fd = other_sock;

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, other_sock, &event)) {
		std::cerr << "Error in epoll_ctl()" << std::endl;
		close(epoll_fd);
		return 1;
	}

	while (1) {
		std::cout << "\n+++++++ Waiting for new connection ++++++++\n\n" << std::endl;
		event_count = epoll_wait(epoll_fd, events, 5, -1);
		if (event_count == -1)
			std::cerr << "Error in epoll_wait()" << std::endl;

		std::cout << event_count << " ready events" << std::endl;

		for (int i = 0; i < event_count; i++) {
			
			int	new_socket = events[i].data.fd;
			if (events[i].data.fd == sock || events[i].data.fd == other_sock) {
				new_socket = accept(new_socket, (struct sockaddr *) &addr, (socklen_t *) &addrlen);
				if (new_socket == -1) {
					std::cerr << "error in accept" << std::endl;
					return 1;
				}
				setnonblocking(new_socket);
				event.events = EPOLLIN | EPOLLET;
				event.data.fd = new_socket;
				
				if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_socket, &event)) {
					std::cerr << "Error in epoll_ctl()" << std::endl;
					close(epoll_fd);
					return 1;
				}
			}
			else {

				char	buffer[30000] = {0};
				int		valread = recv(events[i].data.fd, buffer, 30000, 0);
				std::cout << buffer << std::endl;
				if (valread == -1)
					std::cerr << "No bytes to read" << std::endl;
				const char *hello = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";
				send(events[i].data.fd, hello, strlen(hello), 0);
				std::cout << "------------------Hello message sent-------------------\n" << std::endl;

				if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL) == -1)
					std::cerr << "Error on poll_delete()" << std::endl;
				close(events[i].data.fd);
			}
		}
	}
	return 0;
}