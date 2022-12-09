#pragma once
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <cstdio>

#include "ARequestHandler.hpp"
#include "Json.hpp"

#define	READ_BUF_SIZE 8000

namespace HTTP {

	class GetHandler : public ARequestHandler {
		public:
			//GetHandler(std::map<std::string, std::string> const& mime);
			virtual ~GetHandler();
			GetHandler( void );
			GetHandler&	operator=(GetHandler const& rhs);

			virtual void execute(Message const& req, int client_fd);

		private:

			//std::map<std::string, std::string> const&	mime;
			/**
			 * @brief Helper function to GET request handler. It first verifies
			 * if the file exists and has the necessary permissions, then, if so,
			 * starts the construction of the HTTP response.
			 * @param req Original request as a Message object.
			 * @param resp HTTP response to be started by this function.
			 * @param path Path of the file to be opened.
			 * @return True if successful; False if any error happens during the
			 * attempt to get the file.
			*/
			FILE*	getFile(Message const& req, std::string& path, int const& client_fd);
			
			/**
			 * @brief Helper function to GET request handler. Continues the response
			 * construction, adding either "content-length" or "transfer-encoding",
			 * depending on the size of the body: If it is too big for a single
			 * buffer read, "transfer-encoding" is set with the value chunked".
			 * Finally the request (or sequence of requests) is sent to the client.
			 * @param ifs file stream of the file to be sent.
			 * @param client_fd File descriptor corresponding to the client who
			 * made the request (used to send the response).
			 * @param resp Final response to be sent to the client.
			*/
			void	contentEncoding(FILE * fp, int client_fd, Message& resp);

			void	dirIndex(Message const& req, int fd, std::string const& path);
			
	};
}
