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

	class Mediator {

		public:
			Mediator(void);

			void		method_choice(Message& req, int client_fd);
			std::string	get_date(time_t now);
			std::map<std::string, std::string> mime;
		private:
			
			void	cgi_dealer(Message const& req, int client_fd);
			void	get(Message const& req, int client_fd);
			void	post(Message & req, int client_fd);
			void	del(Message const& req,int client_fd);

			bool	get_file(Message const& req, Message& resp, std::string& path);
			void	content_encoding(std::fstream & ifs, int client_fd, Message& resp);
	};
}
