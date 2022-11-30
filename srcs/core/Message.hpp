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

namespace HTTP {

	class Message {

		public:
			Message(void);
			Message(Message const& cpy);
			~Message(void);
			Message&	operator=(Message const& rhs);
			
			/**
			 * @brief Prints the content of the header map (debug use).
			*/
			void	printMap(void) const;

			/**
			 * @brief Parses an incoming request, separating the method
			 * form the headers and body and organize them in their specific
			 * structure.
			 * @param request request received by the server.
			 * @return	0 if successful, -1 if the request is empty.
			*/
			int							init(std::string & req);

			/**
			 * @brief Method getter.
			 * @return	Returns a reference to the metohd vector of strings.
			*/
			std::vector<std::string> const&	getMethod(void) const;

			/**
			 * @brief Getter for a mapped value in the headers map.
			 * @param key Key value to look for in the map (name of a header).
			 * @return	Returns a copy of mapped value if successful, if not, returns
			 * an empty string.
			*/
			std::string					getHeaderVal(std::string const& key) const;
			
			/**
			 * @brief Adds a new key-value pair to the headers map.
			 * @param key Key value.
			 * @param value Mapped value.
			*/
			void						add(std::string key, std::string value);
			
			/**
			 * @brief Allows to concatenate a string value to a mapped
			 * value corresponding to a specific key.
			 * @param key Key value.
			 * @param value Mapped value.
			*/
			void						addToVal(std::string const& key, std::string const& value_to_add);
			
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
			void						createMethodVec(std::string const& str);
			
			/**
			 * @brief Used when the Message object is a response. Constructs a
			 * string object with the correct format to be sent to a user-agent
			 * based on the response string, the headers map and the body (if
			 * present)
			 * @return The string created.
			*/
			std::string					responseString(void);

			/**
			 * @brief Getter for the body of a request.
			 * @return	Returns a copy of mapped value if successful, if not, returns
			 * an empty string.
			*/
			std::string const&			getBody(void) const;
			
			/**
			 * @brief setter for the body of a request, gets its value and saves it
			 * internally in the object.
			 * @param bod body of the original request constructed as a string.
			*/
			void						setBody(std::string bod);

			JSON::JsonToken * conf;
			std::map<std::string, std::string>	headers; //Max size of the headers section: 8k
		private:

			std::vector<std::string>			method; //Max size: 8k
			std::string							_body;
			
			/**
			 * @brief Helper function, creates a key-value pair based on one header
			 * line of the original request string and adds it to the header map.
			 * @param str string to be split in key and value pair.
			*/
			void	strToHeaderPair(std::string str);
			
			/**
			 * @brief Removes starting and trailing whitespaces in a header string.
			 * @param str Reference to the string to remove from which whitespaces 
			 * will be removed.
			*/
			void	owsTrimmer(std::string& str);
			
	};

	/**
	 * @brief Produces an int value based on a string representation, using the
	 * stringstream method.
	 * @param str Original string.
	 * @return If successful, return the correct int. Undefined behaviour if the
	 * string is poorly formatted.
	*/
	int	ftStoi(std::string const& str);
}
