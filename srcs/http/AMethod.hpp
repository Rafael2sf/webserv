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
		 * @brief CGI handler specific for GET requests that need to be
		 * treated by a CGI (html forms with GET).
		 * @param client http client class
		*/
		void	cgi(Client & client);
		
		int		_confCheck(Client & client);

		void	_wait(Client & client);

		enum
		{
			C_OK = 0,
			C_SERVER_ERROR = 1,
			C_FILE_NOT_FOUND = 2,
			C_FORBIDDEN = 3,
			C_BAD_REQUEST = 4,
		};

	};
}
