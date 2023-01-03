#include "webserv.hpp"

namespace HTTP
{
	std::string getFileExtension(std::string const& str)
	{
		size_t index = str.find_last_of('.');
		if (index == std::string::npos
			|| str.find('/', index) != std::string::npos)
			return "";
		return std::string(str.substr(index + 1));
	}
}
