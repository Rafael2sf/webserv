#pragma	once

#include <map>
#include <vector>
#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <utility>
#include <algorithm>
#include "webserv.hpp"

namespace HTTP
{
	class Client;

	/**
	 * @brief Holds all the information provided by an HTTP request/response.
	*/	
	class Message
	{
		public:
			~Message(void);
			Message(void);
			Message(Message const& cpy);
			Message &operator=(Message const& rhs);
			
			/**
			 * @brief Prints the content of the header map (debug use).
			*/
			void printMap(void) const;

			/**
			 * @brief Method getter.
			 * @return	Returns a reference to the metohd vector of strings.
			*/
			std::vector<std::string> const&	getMethod(void) const;

			// std::string & getMethodAt( size_t i );

			/**
			 * @brief Getter for a mapped value in the headers map.
			 * @param key Key value to look for in the map (name of a header).
			 * @return	Returns a pointer to the mapped value if successful,
			 * otherwise, null.
			*/
			std::string * getField(std::string const& key);
			std::string const* getField(std::string const& key) const;

			/**
			 * @brief Adds a new key-value pair to the headers map.
			 * @param key Key value.
			 * @param value Mapped value.
			*/
			void setField(std::string const& key, std::string const& val);

			/**
			 * @brief Creates the vector based on the method string. The vector
			 * will have 4 elements:
			 * 0 - Request method;
			 * 1 - Path of request as received by the server (without values
			 * preceded by a '?' if they exist);
			 * 2 - HTTP protocol;
			 * 3 - Values preceded by a '?', WITHOUT including the '?'.
			 * @param str Base method string.
			*/
			void createMethodVec(std::string const& str);

			/**
			 * @brief Used when the Message object is a response. Constructs a
			 * string object with the correct format to be sent to a user-agent
			 * based on the response string, the headers map and the body (if
			 * present)
			 * @return The string created.
			*/
			std::string toString(void);
			std::string	body;
			friend class Client;
			int		content_length;

		private:
			size_t	header_bytes;
			std::map<std::string, std::string>	headers; //Max size of the headers section: 8k
			std::vector<std::string>			method; //Max size: 8k

			/**
			 * Clears all the internal memory, making the class reusable.
			*/
			void	clear();
	};

	/**
	 * @brief Produces an int value based on a string representation, using the
	 * stringstream method.
	 * @param str Original string.
	 * @return If successful, return the correct int. Undefined behaviour if the
	 * string is poorly formatted.
	*/
	// int	ftStoi(std::string const& str);
	// std::string	ftItos(int const& n);
}
