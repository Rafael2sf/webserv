#pragma	once

#include <map>
#include <string>
#include <ctime>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "Message.hpp"
#include "webserv.hpp"
#include "GetHandler.hpp"
#include "DelHandler.hpp"
#include "PostHandler.hpp"


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
			
			std::map<std::string, std::string>	mime;
			std::map<int, std::string>			errorText;
		
		private:

			void	errorPage(Message const& req, int fd, int code);

	};
}
