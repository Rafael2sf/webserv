#pragma once

#include <cstring>
#include <list>
#include <vector>
#include <stdexcept>
#include "webserv.hpp"
#include "JsonToken.hpp"

namespace ft
{
	class Json
	{
	public:
		typedef std::list<JsonToken *>::iterator
			iterator;
		typedef std::list<JsonToken *>::const_iterator
			const_iterator;
		typedef std::list<JsonToken *>::reverse_iterator
			reverse_iterator;
		typedef std::list<JsonToken *>::const_reverse_iterator
			const_reverse_iterator;

		~Json();
		Json( void );
		Json( Json const& other );
		Json & operator=( Json const& other );
		
		int parse( char const* filepath );
		
		void cout( void ) const;

		static char const*
		 getStringOf( JsonToken const* t );
		static bool const&
		 getBooleanOf( JsonToken const* t );
		static int const&
		 getIntegerOf( JsonToken const* t );
		static std::vector<JsonToken*> const&
		 getObjectOf( JsonToken const* t );

		void clear(void);

		std::list<JsonToken *> tokens;
	};
}
