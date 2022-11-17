#pragma once

extern "C"
{
	#include <netinet/in.h>
	#include <fcntl.h>
	#include <unistd.h>
}
//#include <algorithm>
#include <cstring>
#include <list>
#include "webserv.hpp"

namespace ft
{
	/**
	 * @brief Holds all the information related to a
	 *  listening sock and it's port.
	*/
	typedef struct s_sock_info
	{
		int			fd;
		int			port;
		sockaddr_in	addr;
	}				t_sock_info;

	/**
	 * @brief Store and manipulate all listening sockets,
	 *  using a linked list of t_sock_info structures.
	*/
	class HTTPSocks
	{
		public:
		~HTTPSocks();
		HTTPSocks( void );
		HTTPSocks( HTTPSocks const& other );
		HTTPSocks & operator=( HTTPSocks const& rhs );

		std::list<t_sock_info> list;

		/**
		 * @brief Creates a t_socket_info structure in a ready to,
		 * accept mode, and inserts it into the internal %list.
		 * @param port the number of the port to listen in
		 * @return
		 * On sucess, returns 0 returns a pointer to
		 * the correspondent t_sock_info, wich is
		 * set to listen on %port, otherwise, NULL and
		 * errno is set to indicate the error.
		*/
		t_sock_info		*insert( uint16_t port );

		/**
		 * @brief Searchs for a t_sock_info in the internal %list,
		 *  by comparing the file descriptor.
		 * @param sock_fd file descriptor to find
		 * @return
		 * On sucess, returns a pointer to the correspondent 
		 * t_sock_info, otherwise, returns NULL
		*/
		t_sock_info	const*	find( int sock_fd ) const;

		/**
		 * @brief Closes all sockets and erases all elements.
		*/
		void	clear( void );
	};
}
