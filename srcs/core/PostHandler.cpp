#include "PostHandler.hpp"

namespace HTTP {

	PostHandler::PostHandler(void): ARequestHandler() {}

	PostHandler::~PostHandler(void) {
	}

	PostHandler::PostHandler(PostHandler const& cpy): ARequestHandler() { (void)cpy; };

	PostHandler&	PostHandler::operator=(PostHandler const& rhs) {
		(void)rhs;
		return *this;
	};

	void		PostHandler::execute(Message const& req, int client_fd) {
		DEBUG2("This is a POST request");

		cgiDealer(req, client_fd);
	};

}