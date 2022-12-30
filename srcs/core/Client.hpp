#pragma once

#include "Sockets.hpp"
#include "webserv.hpp"
#include "Message.hpp"
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

namespace HTTP
{
	enum
	{
		CONNECTED,
		STATUS_LINE,
		HEADER_FIELDS,
		BODY_CONTENT,
		OK,
		REDIRECT,
		SENDING,
		CGI_PIPING,
		CGI_FINISHED,
		SEND_ERROR
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
		JSON::Node *server;
		JSON::Node *location;

		/**
		 * @brief Updates the request message of the client
		 * by reading from the %fd, if the request is complete.
		 * method ok() will return true.
		*/
		int update( Sockets const& sockets );

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


		/**
		 * @brief Helper function to request handling (GET files). It first
		 * verifies if the file exists and has the necessary permissions,
		 * then, if so, starts the construction of the HTTP response.
		 * @param path path of the file to be opened.
		 * @return True if successful; False if any error happens during the
		 * attempt to get the file.
		*/
		bool	getFile( std::string & path);

		/**
		 * @brief Helper function to GET requests. Continues the response
		 * construction, adding either "content-length" or "transfer-encoding",
		 * depending on the size of the body: If it is too big for a single
		 * buffer read, "transfer-encoding" is set with the value "chunked".
		 * Finally the request (or sequence of requests) is sent to the client.
		*/
		int		contentEncoding(void);

		/**
		 * @brief Attempts to create a http response with the files indexed relative
		 * to the -path- if autoindex is set on -location- and -path- is a dir.
		 * @param path search path
		*/
		void	dirIndex(std::string const& path);

		int		clientPipe[2];
		int		cgiSentBytes;
		int		childPid;
		int 	state;
		FILE *	fp;

	private:
			/**
		 * @brief Removes starting and trailing whitespaces in a header string.
		 * @param str Reference to the string to remove from which whitespaces 
		 * will be removed.
		*/
		void _owsTrimmer(std::string& str);
		int _getHostFromUrl( void );
		int _peekHeaderFields( Sockets const& sockts );
		int _updateHeaders( char const* buff, size_t n );
		int _updateStatusLine( char const* buff, size_t n );
		int _updateBody( char const * buff, size_t n, Sockets const& sockets );
		int _validateStatusLine( void );
		std::string const*  _errorPage( int code );
		void _redirect( void );
		void _defaultPage( int code, bool close_connection );
		int _unchunking(void);
	};
}
