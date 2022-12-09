#pragma once
#include "ARequestHandler.hpp"

namespace HTTP {

	class DelHandler : public ARequestHandler {
		public:
			DelHandler(void);
			virtual ~DelHandler();
			DelHandler(DelHandler const& cpy);
			DelHandler&	operator=(DelHandler const& rhs);

			/**
			 * @brief Handler of DELETE requests.
			 * @param req Original request as a Message object.
			 * @param client_fd File descriptor corresponding to the client who
			 * made the request (used to send the response).
			*/
			virtual void execute(Message const& req, int client_fd);
		
		private:
	};
}