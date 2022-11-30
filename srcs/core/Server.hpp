#pragma once

#include <cerrno>
#include <cstdlib>
#include "Sockets.hpp"
#include "Epoll.hpp"
#include "Message.hpp"
#include "Mediator.hpp"
#include "webserv.hpp"
#include "Client.hpp"
#include "Json.hpp"

#define	RECEIVE_BUF_SIZE 30000

namespace HTTP
{
	/*
		A wrapper class to all the functionality of the weberver,
		meant to be used by the final user.
	*/
	class Server
	{
		public:

		~Server();
		Server( void );
		Server( Server const& other );
		Server & operator=( Server const& rhs);

		/**
		 * @brief Handles the reception, treatment and response
		 * to a request.
		 * @param i File descriptor of the client;
		 * @param med Object that distributes requests based on
		 * the method stated.
		*/
		void ft_handle(Client & cli, int i, Mediator & med);

		Sockets		socks;
		Epoll	epoll;
		JSON::Json	conf;
		static int	state;
		std::map<int, Client> clients;

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
		void	check_times(void);
	};
}
