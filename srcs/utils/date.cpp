#include "webserv.hpp"
#include <cstring>
#include <ctime>

namespace HTTP
{
	#define	DATE_BUF_SIZE 40
	/**
	 * @brief Creates a string of the date based on the date given as
	 * an argument, with the ideal format for HTTP requests/responses.
	 * @param tm_info time_t structure used for the string's creation.
	 * @return The constructed string.
	*/
	std::string getDate(time_t const& tm_info)
	{
		tm*		current_time;
		char	buffer[DATE_BUF_SIZE];
		std::string	ret;

		memset(buffer, 0, DATE_BUF_SIZE);
		current_time = localtime(&tm_info);
		strftime(buffer, DATE_BUF_SIZE, "%a, %d %b %Y %X %Z",current_time);
		ret = buffer;
		return ret;
	};
}
