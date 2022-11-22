#pragma once
#include "webserv.hpp"
#include "HTTPReq.hpp"
#include <vector>

namespace ft
{
	class CGI
	{
		public:

			~CGI();
			CGI( void );
			CGI(HTTPReq const& req);
			CGI( CGI const& other );
			CGI & operator=( CGI const& rhs);

			char**	getEnv(void);
			char**	getArgs(void); 

		private:

			char**	env;
			char**	args;
	};
}