#pragma once

#include "webserv.hpp"
#include "Message.hpp"
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

namespace HTTP
{
	enum
	{
		CONNECTED,
		STATUS_LINE,
		HEADER_FIELDS,
		BODY_CONTENT,
		OK,
	};

	class Client
	{
	public:
		~Client( void );
		Client( void );
		Client( Client const& other );
		Client & operator=( Client const& rhs );

		Message		req;
		Message		res;
		int			fd;
		struct \
		sockaddr_in	ai;
		double		timestamp;
		JSON::Node *config;

		/**
		 * @brief Updates the request message of the client
		 * by reading from the %fd, if the request is complete.
		 * method ok() will return true.
		*/
		int update( void );

		/**
		 * @brief return true if the request message has
		 * been fully parsed and the http client is
		 * waiting for a response.
		*/
		bool ok( void );

		/**
		 * @brief Clears all the internal memory used,
		 * except for %fd and %timestamp allowing class to be reused.
		*/
		void reset( void );
		
		/**
		 * @brief Sends a predefined error http response.
		 * to the client.
		 * @param code error code  
		*/
		void error(int code);

	private:
			/**
		 * @brief Removes starting and trailing whitespaces in a header string.
		 * @param str Reference to the string to remove from which whitespaces 
		 * will be removed.
		*/
		void	owsTrimmer(std::string& str);
		int state;
		int _updateHeaders( char const* buff, size_t n );
		int _updateStatusLine( char const* buff, size_t n );
		int _updateBody( char const * buff, size_t n );
	};
}
