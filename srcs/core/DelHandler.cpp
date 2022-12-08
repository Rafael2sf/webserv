#include "DelHandler.hpp"

namespace HTTP {

	DelHandler::DelHandler(void): ARequestHandler() {}

	DelHandler::~DelHandler(void) {
	}

	DelHandler::DelHandler(DelHandler const& cpy): ARequestHandler() { (void)cpy; };

	DelHandler&	DelHandler::operator=(DelHandler const& rhs) {
		(void)rhs;
		return *this;
	};

	void		DelHandler::execute(Message const& req, int client_fd) {
		DEBUG2("This is a DELETE request");

		cgiDealer(req, client_fd);
	};

}