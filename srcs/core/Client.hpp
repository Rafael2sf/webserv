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
		SENDING,
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
		 * @brief set the state to OK.
		*/
		void setOk( void );
		
		/**
		 * @brief returns true if the server finished responding to
		 * a request(used for chunked request sending).
		*/
		bool sending( void );

		/**
		 * @brief set the state to SEND.
		*/
		void setSending( void );

		/**
		 * @brief Clears all the internal memory used,
		 * except for %fd and %timestamp allowing class to be reused.
		*/
		void reset( void );
		
		/**
		 * @brief Sends a predefined error http response.
		 * to the client.
		 * @param code error code  
		 * @param close_connection if request asks to keep
		 * connection alive, and this is set to true,
		 * connecton will still be closed.
		*/

		void error(int code, bool close_connection);
		void print_message( Message const& m, std::string const& s );

		FILE *			fp;

	private:
		int state;
			/**
		 * @brief Removes starting and trailing whitespaces in a header string.
		 * @param str Reference to the string to remove from which whitespaces 
		 * will be removed.
		*/
		void _owsTrimmer(std::string& str);
		int _updateHeaders( char const* buff, size_t n );
		int _updateStatusLine( char const* buff, size_t n );
		int _updateBody( char const * buff, size_t n );
		int _validateStatusLine( void );
	};
}
