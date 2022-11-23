#pragma	once

#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "HTTPReq.hpp"
#include "webserv.hpp"
#include "CGI.hpp"

namespace ft {

	class Mediator {

		public:
			Mediator(void);

			void		method_choice(HTTPReq& req, int client_fd);
			std::string	get_date(time_t now);
		private:
			
			void	cgi_dealer(HTTPReq const& req, int client_fd);
			void	get(HTTPReq const& req, int client_fd);
			void	post(HTTPReq & req, int client_fd);
			void	del(HTTPReq const& req,int client_fd);

			bool	get_file(HTTPReq const& req, HTTPReq& resp, std::string& path);
			void	content_encoding(std::fstream & ifs, int client_fd, HTTPReq& resp);
	};
}
