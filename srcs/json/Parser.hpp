#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <algorithm>
#include <utility>
#include "Json.hpp"

namespace JSON
{
	class Parser
	{
		public:
			~Parser();
			Parser( void );
			void clear( void );
			void reset( void );
			Node * build( std::string const& str );
			std::pair<size_t, size_t> errPos( void ) const;

		private:
			Parser & operator=( Parser const& other );
			Parser( Parser const& other );

			void		_readObject( Node * parent );
			void		_readPair( Node * parent );
			void		_readArray( void );
			void		_readValue( std::string const& property, Node * parent );
			void		_ignoreSpaces( bool throw_except );
			int			_isSpace( char c ) const;

			size_t						_row;
			std::string::const_iterator	_lf;
			size_t						_depth;
			Node						*_cur;
			Node						*_root;
			std::string::const_iterator	_beg;
			std::string::const_iterator	_end;
	};
}
