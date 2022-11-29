#pragma once

extern "C"
{
	#include <netinet/in.h>
	#include <fcntl.h>
	#include <unistd.h>
}
//#include <algorithm>
#include <map>
#include <cstring>
#include <list>
#include "webserv.hpp"

namespace HTTP
{
	/**
	 * @brief Holds all the information related to a
	 *  listening sock and it's port.
	*/
	typedef struct s_sock_info
	{
		int						fd;
		int						host;
		int						port;
		sockaddr_in				addr;
		JSON::JsonToken * 		conf;
		std::map<int, double>	clients;
	}							t_sock_info;

	/**
	 * @brief Store and manipulate all listening sockets,
	 *  using a linked list of t_sock_info structures.
	*/
	class Sockets
	{
		public:
		~Sockets();
		Sockets( void );
		Sockets( Sockets const& other );
		Sockets & operator=( Sockets const& rhs );

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
		t_sock_info const*	insert( JSON::JsonToken * block );

		/**
		 * @brief Searchs for a t_sock_info in the internal %list,
		 *  by comparing the file descriptor.
		 * @param sock_fd file descriptor to find
		 * @return
		 * On sucess, returns a pointer to the correspondent 
		 * t_sock_info, otherwise, returns NULL
		*/
		t_sock_info	*	findByFd( int sock_fd );

		/**
		 * @brief Searchs for a t_sock_info in the internal %list,
		 *  by comparing the file descriptor.
		 * @param port port to find
		 * @return
		 * On sucess, returns a pointer to the correspondent 
		 * t_sock_info, otherwise, returns NULL
		*/
		t_sock_info	const*	findByPort( int port ) const;

		/**
		 * @brief Closes all sockets and erases all elements.
		*/
		void	clear( void );
	};
}
