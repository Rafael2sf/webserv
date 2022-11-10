#pragma	once

#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include "HTTPReq.hpp"

namespace ft {

	class Mediator {

		public:
			Mediator(void);

			std::string	method_choice(HTTPReq const& req);
		private:
			
			std::string	get(HTTPReq const& req);
			std::string	post(HTTPReq const& req);
			std::string	del(HTTPReq const& req);

			std::string	get_file(HTTPReq const& req, std::string& path);
	};
}