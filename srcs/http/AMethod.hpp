#pragma once

#include <iostream>
#include <string>
#include <map>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <functional>

#include "CGI.hpp"
#include "Client.hpp"

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
		 * @brief 
		 * Handler specific for requests that need to be
		 * treated by a CGI.
		 * @param client http client class
		*/
		void	cgi(Client & client, std::string const& path);
		
		int		_confCheck(Client & client);

	};
}
