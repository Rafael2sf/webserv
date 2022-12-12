#pragma once

#include <iostream>
#include <string>
#include <ctime>
#include <map>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <functional>

#include "CGI.hpp"
#include "Client.hpp"

#define	DATE_BUF_SIZE 40

namespace HTTP
{
	class AMethod : public std::unary_function<Client, void>
	{
		public:
		virtual ~AMethod(void);
		AMethod(void);
		AMethod(AMethod const& cpy);
		AMethod&	operator=(AMethod const& rhs);

		virtual void	response(Client & client) = 0;

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
		 * @param client http client class
		*/
		void	cgi(Client & client);
	};
}
