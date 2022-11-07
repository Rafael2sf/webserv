#pragma once

#include <cerrno>
#include "HTTPSocks.hpp"
#include "HTTPEpoll.hpp"

namespace ft
{
	/*
		A wrapper class to all the functionality of the weberver,
		meant to be used by the final user.
	*/
	class HTTPServer
	{
		public:

		~HTTPServer();
		HTTPServer( void );
		HTTPServer( HTTPServer const& other );
		HTTPServer & operator=( HTTPServer const& rhs);

		void	ft_handle(int i);

		HTTPSocks	socks;
		HTTPEpoll	epoll;

		/**
		 * @brief Initiates the server with default configurations.
		 * @return
		 * On success, returns 0. Otherwise, -1 and
		 * errno is set to indicate the error.
		*/
		int		init( void );

		/**
		 * @brief Initiates the server after parsing %filepath.
		 * @param filepath server configuration file.
		 * @return
		 * On success, returns 0. Otherwise, -1 and
		 * errno is set to indicate the error.
		*/
		int		init( char const* filepath );

		/**
		 * @brief Server main loop for accepting and
		 *  handling socket connections.
		 * 
		 * @ Server must be initialized before looping
		*/
		void	loop( void );

		private:
	};
}
