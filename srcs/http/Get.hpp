#pragma once
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <cstdio>

#include "AMethod.hpp"
#include "Json.hpp"

namespace HTTP
{
	class Get : public AMethod
	{
		public:
			virtual ~Get();
			Get( void );
			Get&	operator=(Get const& rhs);

			/**
			 * @brief Handler of DELETE requests.
			 * @param Client http client class
			*/
			void response(Client & client);

		private:

			/**
			 * @brief Helper function to GET request handler. It first verifies
			 * if the file exists and has the necessary permissions, then, if so,
			 * starts the construction of the HTTP response.
			 * @param client http client class
			 * @param path path of the file to be opened.
			 * @return True if successful; False if any error happens during the
			 * attempt to get the file.
			*/
			FILE*	getFile( Client & client, std::string & path, JSON::Node * location );

			/**
			 * @brief Helper function to GET request handler. Continues the response
			 * construction, adding either "content-length" or "transfer-encoding",
			 * depending on the size of the body: If it is too big for a single
			 * buffer read, "transfer-encoding" is set with the value chunked".
			 * Finally the request (or sequence of requests) is sent to the client.
			 * @param clien http client class
			 * @param fp file to read from
			*/
			void	contentEncoding(Client & client);

			/**
			 * @brief Attempts to create a http response with the files indexed relative
			 * to the -path- if autoindex is set on -location- and -path- is a dir.
			 * @param client http client class
			 * @param path search path
			 * @param location location config block 
			*/
			void	dirIndex(Client & client, std::string const& path, JSON::Node * location);
	};
}
