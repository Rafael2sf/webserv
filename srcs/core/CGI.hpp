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
			CGI( void );
			CGI(Message const& req);
			CGI( CGI const& other );
			CGI & operator=( CGI const& rhs);

			char**	getEnv(void);
			char**	getArgs(void); 

		private:

			char**	env;
			char**	args;
	};
}
