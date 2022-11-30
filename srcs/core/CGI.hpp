#pragma once
#include "webserv.hpp"
#include "Message.hpp"
#include <vector>

namespace HTTP
{
	class CGI
	{
		public:

			~CGI();
			
			/**
			 * @brief Constructs a CGI object with the necessary environment
			 * variables and arguments to be passed to the execve() function,
			 * independently of the CGI method used.
			 * @param req HTTP request which will be responded to by the CGI.
			*/
			CGI(Message const& req);

			/**
			 * @brief Getter for the CGI environment variables.
			 * @return Array of char strings containing the variables.
			*/
			char**	getEnv(void);
			
			/**
			 * @brief Getter for the CGI arguments.
			 * @return Array of char strings containing the arguments.
			*/
			char**	getArgs(void); 

		private:

			CGI( void );
			CGI( CGI const& other );
			CGI & operator=( CGI const& rhs);
			
			char**	env;
			char**	args;
	};
}
