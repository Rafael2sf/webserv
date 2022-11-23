/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPEpoll.hpp                                      :+:      :+:    :+:   */
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


#define FT_ERROR(A, B) std::cerr << A << ": " << B << strerror(errno) << std::endl
#define EPOLL_SIZE 5
#define EPOLL_TIMEOUT 5000

namespace ft {
	class HTTPSocks;

	class HTTPEpoll {

		public:

			HTTPEpoll(void);
			~HTTPEpoll(void);
			HTTPEpoll(HTTPEpoll const& cpy);

			/*Methods*/

			
			/**
			 * @brief Creates an epoll instance to monitor incoming connections.
			 * @param socks Contains the opened sockets where connections will come
			 * from.
			 * @return
			 * On sucess, returns 0 and all opened sockets will be monitored by the
			 * struct "events" present in this object, otherwise, -1 and
			 * errno is set to indicate the error.
			*/
			int	init(HTTPSocks const& socks);
			
			/**
			 * @brief Inserts an incoming event fd in the "events" struct, allowing
			 * its use to read/write. Flags are set appropriately.
			 * @param sofd file descriptor of the socket from wich the event is coming from.
			 * @return
			 * On sucess, returns 0 and the event fd is added to the list of fds to be
			 * monitored, otherwise, -1 and
			 * errno is set to indicate the error.
			*/
			int insert( int sofd);
			
			/**
			 * @brief Erases an event fd from the "events" struct and closes it.
			 * This function should be called after the event has been handled.
			 * @param ev_index index of the fd of the event to be deleted. This index
			 * corresponds to a position in the internal array of the struct "events".
			 * @return
			 * On sucess, returns 0 and the event fd is removed and closed,
			 *  otherwise, -1 and errno is set to indicate the error.
			*/
			int	erase(int ev_index);

			/**
			 * @brief Abstracts the epoll_wait() function, which waits for
			 * incoming events through the defined fds of interest.
			 * @return
			 * On sucess, returns the amount of events waiting for processing,
			 * otherwise, -1 and errno is set to indicate the error.
			*/
			int	wait(void);

			struct epoll_event	events[EPOLL_SIZE];
		
		private:
			int					fd;
			int					size;
			
	};
}
