#pragma once

extern "C"
{
	#include <netinet/in.h>
	#include <fcntl.h>
	#include <unistd.h>
}
//#include <algorithm>
// #include <map>
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
		int				fd;
		sockaddr_in		addr;
		JSON::Node *	config;
	}					t_sock_info;

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
		 * @param host the adress to listen
		 * @param port the port to listen
		 * @return
		 * On sucess, returns a pointer to
		 * the correspondent t_sock_info, wich will be set
		 * set to listen on %host:%port, otherwise, NULL and
		 * errno is set to indicate the error.
		*/
		t_sock_info * insert( u_int32_t host, u_int16_t port );

		/**
		 * @brief Initiates all sockets previously added to the internal list
		 * @return
		 * On sucess, returns 0, otherwise removes all sockets, returns -1 and errno is
		 * set to indicate the error.
		*/
		int listen( void );

		/**
		 * @brief Searchs for a t_sock_info in the internal %list,
		 *  by comparing the file descriptor, which can be either a
		 * server sock or a client sock.
		 * @param sock_fd file descriptor to find
		 * @return
		 * On sucess, returns a pointer to the correspondent 
		 * t_sock_info, otherwise, returns NULL
		*/
		t_sock_info	* find( int sock_fd );

		/**
		 * @brief Closes all sockets and erases all elements.
		*/
		void clear( void );

	private:
	};
}
