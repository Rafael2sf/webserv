#include "webserv.hpp"
#include <sstream>

namespace HTTP
{
	/**
	 * @brief Produces an int value based on a string representation, using the
	 *  stringstream method.
	 * @param str Original string.
	 * @return If successful, return the correct int. Undefined behaviour if the
	 * string is poorly formatted.
	*/
	int	stoi(std::string const& str, std::ios_base & (&f)(std::ios_base &))
	{
		std::stringstream	ss;
		int					ret = 0;
		
		ss << f << str;
		ss >> ret;
		return ret;
	}

	/**
	 * @brief Produces an string value based on a integer, using the
	 *  stringstream method.
	 * @param n Original integer.
	 * @return formated string
	*/
	std::string	itos(int const& n, std::ios_base & (&f)(std::ios_base &))
	{
		std::stringstream	ss;
		std::string			ret;

		ss << f << n;
		ss >> ret;
		return ret;
	}
}
