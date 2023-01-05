#pragma once

#include "Sockets.hpp"
#include "Epoll.hpp"
#include "webserv.hpp"
#include "Client.hpp"
#include "Json.hpp"
#include "AMethod.hpp"

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

		static int state;
		static std::map<std::string, std::string> mime;
		static std::map<int, std::string> error;
		static std::map<pid_t, int>	childProcInfo;

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

		void clear( void );
	private:
		Server( Server const& other );
		Server & operator=( Server const& rhs);

		Sockets socks;
		Epoll epoll;
		JSON::Json config;
		std::map<int, Client> clients;

		/**
		 * @brief Initializes the mime map
		*/
		void _init_mimes( void );

		/**
		 * @brief Initializes the errors map
		*/
		void _init_errors( void );

		/**
		 * @brief Everytime a loop occurs, this
		 *  functions is called to check for clients
		 * that exceeded S_CONN_TIMEOUT since their last
		 * event proceeded by removing them. It also checks
		 * for existing child processes and their deaths to
		 * update related clients and deal with possible connection
		 * changes.
		*/
		void _timeoutChildPrune(void);

		/**
		 * @brief Accepts a client connection on
		 * a server socket %si.
		*/
		void _accept( t_sock_info const& si );

		/**
		 * @brief Updates an existing connection
		 * by veryfing its state.
		*/
		void _update( int socket, int epoll_index );

		/**
		 * @brief Handles the creation of the http response
		 * @param client http client class
		*/
		void _handle( Client & client );

		/**
		 * @brief Chooses and launches the correct method handler
		 * @param client http client class
		*/
		void _methodChoice( Client & client );

		void _updateConnection( Client & client );
	};

	JSON::Node *matchServer(Sockets const& so, Client const &cli);
	JSON::Node *matchLocation(JSON::Node *serv, std::string const &path);
	int	validateConfig( JSON::Json const& json );
}
