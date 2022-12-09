#pragma once
#include "ARequestHandler.hpp"

namespace HTTP {

	class PostHandler : public ARequestHandler {
		public:
			PostHandler(void);
			virtual ~PostHandler();
			PostHandler(PostHandler const& cpy);
			PostHandler&	operator=(PostHandler const& rhs);

			/**
			 * @brief Handler of POST requests.
			 * @param req Original request as a Message object.
			 * @param client_fd File descriptor corresponding to the client who
			 * made the request (used to send the response).
			*/
			virtual void execute(Message const& req, int client_fd);
		
		private:
	};
}