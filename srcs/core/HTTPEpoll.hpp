/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPEpoll.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: daalmeid <daalmeid@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/10/31 14:16:23 by daalmeid          #+#    #+#             */
/*   Updated: 2022/11/07 14:59:58 by daalmeid         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma	once
#include <sys/epoll.h>
#include <iostream>
#include <cerrno>
#include <cstring>
#include <unistd.h>
#include "HTTPSocks.hpp"

#define FT_ERROR(A, B) std::cerr << A << ": " << B << strerror(errno) << std::endl
#define EPOLL_SIZE 5
#define EPOLL_TIMEOUT -1

namespace ft {

	class HTTPEpoll {

		public:

			HTTPEpoll(void);
			HTTPEpoll(HTTPEpoll const& cpy);

			/*Methods*/

			int	init(HTTPSocks const& socks);
			int insert( int __sofd);
			int	erase(int ev_index);
			int	wait(void);

			struct epoll_event	events[EPOLL_SIZE];
		
		private:
			int					fd;
			int					size;
			
	};
}
