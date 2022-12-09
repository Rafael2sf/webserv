#pragma once
#include <iostream>
#include <string>
#include <ctime>
#include <map>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include "Message.hpp"
#include "CGI.hpp"

#define	DATE_BUF_SIZE 40

namespace HTTP {
	class ARequestHandler {

		public:

			ARequestHandler(void);
			ARequestHandler(ARequestHandler const& cpy);
			virtual ~ARequestHandler(void);
			ARequestHandler&	operator=(ARequestHandler const& rhs);


			virtual void	execute(Message const& req, int client_fd) = 0;
		
		protected:

			/**
			 * @brief Creates a string of the date based on the date given as
			 * an argument, with the ideal format for HTTP requests/responses.
			 * @param tm_info time_t structure used for the string's creation.
			 * @return The constructed string.
			*/
			std::string	getDate(time_t const& tm_info);
			
			/**
			 * @brief CGI handler specific for GET requests that need to be
			 * treated by a CGI (html forms with GET).
			 * @param req Original request as a Message object.
			 * @param client_fd File descriptor corresponding to the client who
			 * made the request (used to send the response).
			*/
			void	errorPage(Message const& req, int fd, int code);
			
			/**
			 * @brief CGI handler specific for GET requests that need to be
			 * treated by a CGI (html forms with GET).
			 * @param req Original request as a Message object.
			 * @param client_fd File descriptor corresponding to the client who
			 * made the request (used to send the response).
			*/
			void	cgiDealer(Message const& req, int client_fd);

			
			//std::map<int, std::string>			errorText;
	};
}
