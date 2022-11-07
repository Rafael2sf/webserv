#include <map>
#include <iostream>
#include <string>
#include <cstring>
#include <utility>


namespace ft {

	class HTTPReq {

		public:
			HTTPReq(void);
			HTTPReq(char* request);
			HTTPReq(HTTPReq const& cpy);
			~HTTPReq(void);
			HTTPReq&	operator=(HTTPReq const& rhs);

			void		print_map(void) const;
			void		pair_create_add(std::string str);
			std::string	wp_trimmer(std::string const& str);

		private:
			std::map<std::string, std::string>	headers;
			std::string							method;
	};
}
