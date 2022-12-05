#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include "webserv.hpp"
#include "Node.hpp"

namespace JSON
{
	class Node;

	class Json
	{
	public:
		~Json();
		Json( void );
	
		std::string const& err( void ) const;
		bool empty( void ) const;
		int from( char const* filepath );
		void cout( void ) const;
		void clear(void);

		Node *		tokens;
		void begin( void );
		void end( void );
	private:
		Json( Json const& other );
		Json & operator=( Json const& other );
		std::string	_err;
	};

	class	ParseError: public std::exception
	{
	private:
		std::string _err;
	public:
		~ParseError() throw();
		ParseError( char const* e );
		virtual const char *what() const throw();
	};
}
