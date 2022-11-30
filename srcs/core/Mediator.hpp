#pragma	once

#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "Message.hpp"
#include "webserv.hpp"
#include "CGI.hpp"

namespace HTTP {

	/**
	 * @brief Middleman of the server, receives a request and handles it
	 * depending on the request method.
	*/
	class Mediator {

		public:
			Mediator(void);

			/**
			 * @brief Redirects the request to the correct handler depending on
			 * its method.
			 * @param req Original request as a Message object.
			 * @param client_fd  File descriptor corresponding to the client who
			 * made the request (used to send the response).
			*/
			void		methodChoice(Message& req, int client_fd);
			
			/**
			 * @brief Creates a string of the date based on the date given as
			 * an argument, with the ideal format for HTTP requests/responses.
			 * @param tm_info time_t structure used for the string's creation.
			 * @return The constructed string.
			*/
			std::string	getDate(time_t const& tm_info);
			
			std::map<std::string, std::string> mime;
		
		private:
			
			/**
			 * @brief CGI handler specific for GET requests that need to be
			 * treated by a CGI (html forms with GET).
			 * @param req Original request as a Message object.
			 * @param client_fd File descriptor corresponding to the client who
			 * made the request (used to send the response).
			*/
			void	cgiDealer(Message const& req, int client_fd);
			
			/**
			 * @brief Handler of GET requests.
			 * @param req Original request as a Message object.
			 * @param client_fd File descriptor corresponding to the client who
			 * made the request (used to send the response).
			*/
			void	get(Message const& req, int client_fd);
			
			/**
			 * @brief Handler of POST requests.
			 * @param req Original request as a Message object.
			 * @param client_fd File descriptor corresponding to the client who
			 * made the request (used to send the response).
			*/
			void	post(Message & req, int client_fd);
			
			/**
			 * @brief Handler of DELETE requests.
			 * @param req Original request as a Message object.
			 * @param client_fd File descriptor corresponding to the client who
			 * made the request (used to send the response).
			*/
			void	del(Message const& req,int client_fd);

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
			bool	getFile(Message const& req, Message& resp, std::string& path);
			
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
			void	contentEncoding(std::fstream & ifs, int client_fd, Message& resp);
	};
}
