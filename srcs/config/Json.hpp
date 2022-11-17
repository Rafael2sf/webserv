#pragma once

#include <cstring>
#include <list>
#include <vector>
#include "webserv.hpp"
#include "JsonToken.hpp"

namespace ft
{
	class Json
	{
	public:
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

		std::list<JsonToken *> tokens;
	};
}
