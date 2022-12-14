/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Epoll.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: daalmeid <daalmeid@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/10/31 14:16:23 by daalmeid          #+#    #+#             */
/*   Updated: 2022/11/21 19:45:26 by daalmeid         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma	once

#include <sys/epoll.h>
#include <iostream>
#include <cerrno>
#include <cstring>
#include <unistd.h>
#include "webserv.hpp"
#define	LISTEN_SOCKET 0
#define	CLIENT_CONNECT 1

namespace HTTP {
	class Sockets;

	class Epoll {

		public:

			~Epoll(void);
			Epoll(void);

			/**
			 * @brief Calls operator[] on epoll events array
			 * @param index
			 * @return 
			 * returns epoll_event at %index position
			*/
			epoll_event & operator[](size_t index);

			/**
			 * @brief Creates an epoll instance to monitor incoming connections.
			 * @param socks Contains the opened sockets where connections will come
			 * from.
			 * @return
			 * On sucess, returns 0 and all opened sockets will be monitored by the
			 * struct "events" present in this object, otherwise, -1 and
			 * errno is set to indicate the error.
			*/
			int	init(Sockets const& socks);
			
			/**
			 * @brief Inserts an incoming event fd in the "events" struct, allowing
			 * its use to read/write. Flags are set appropriately.
			 * @param sofd file descriptor of the socket from wich the event is coming from.
			 * @param flag tells if the file descriptor belongs to a socket listen or a client connection.
			 * @return
			 * On sucess, returns 0 and the event fd is added to the list of fds to be
			 * monitored, otherwise, -1 and
			 * errno is set to indicate the error.
			*/
			int insert( int sofd, int flag);

			/**
			 * @brief Erases an event fd from the "events" struct and closes it.
			 * This function should be called after the event has been handled.
			 * @param cli_fd the file descriptor to be removed
			 * @return
			 * On sucess, returns 0 and the event fd is removed and closed,
			 *  otherwise, -1 and errno is set to indicate the error.
			*/
			int	erase(int cli_fd);

			/**
			 * @brief Abstracts the epoll_wait() function, which waits for
			 * incoming events through the defined fds of interest.
			 * @return
			 * On sucess, returns the amount of events waiting for processing,
			 * otherwise, -1 and errno is set to indicate the error.
			*/
			int	wait(void);

		private:
			Epoll(Epoll const& cpy);
			Epoll & operator=(Epoll const& cpy);

			struct epoll_event	events[S_MAX_CLIENT];
			int					fd;
			int					size;
			
	};
}
