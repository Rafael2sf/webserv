#pragma once

#include <utility>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <fstream>
#include <vector>
#include <cstring>
#include <limits>
#include <algorithm>
#include <iostream>

namespace ft
{
	typedef enum e_jtoken_type
	{
		json_none_type,
		json_object_type,
		json_string_type,
		json_integer_type,
		json_boolean_type,
		json_null_type,
		json_array_type,
	}	t_jtoken_type;

	class JsonToken
	{
	public:
		virtual ~JsonToken();
		JsonToken( void );
		JsonToken( JsonToken const& other );
		JsonToken & operator=( JsonToken const& other );
		JsonToken & operator[](char const* key);

		const char*	getProperty	( void ) const;
		void		setProperty	( char * property );
		JsonToken * getParent	( void ) const;
		void		setParent	( JsonToken * token );
		int			getType		( void ) const;

		void print( void ) const;
		JsonToken * find_first( std::string const& key, size_t max_depth = 1);
		template <class T> T as( void ) const
		{
			throw std::invalid_argument("as(): call");
		}

	protected:
		char *			_property;
		JsonToken *		_parent;
		t_jtoken_type	_type;
	};

	typedef struct s_jparser_info
	{
		char *			path;
		char *			bytes;
		char *			cursor;
		char *			property;
		unsigned int	col;
		unsigned int	row;
		unsigned int	 depth;
		bool 			as_comma;
		bool 			as_colon;
		JsonToken *		prev;
		JsonToken *		token;
	}					t_jparser_info;

	class JsonString : public JsonToken
	{
	public:
		~JsonString();
		JsonString( char const* value );
		JsonString( JsonString const& other );
		JsonString & operator=( JsonString const& other );

		char const* getValue( void ) const;
	private:
		JsonString( void );
		char * _data;
	};

	class JsonObject : public JsonToken
	{
	public:
		~JsonObject();
		JsonObject( void );
		JsonObject( JsonObject const& other );
		JsonObject & operator=( JsonObject const& other );

		std::vector<JsonToken *> data;
	private:
	};

	class JsonInteger : public JsonToken
	{
	public:
		~JsonInteger();
		JsonInteger( int value );
		JsonInteger( JsonInteger const& other );
		JsonInteger & operator=( JsonInteger const& other );

		int const& getValue( void ) const;
	private:
		JsonInteger( void );
		int _data;
	};

	class JsonBoolean : public JsonToken
	{
	public:
		~JsonBoolean();
		JsonBoolean( bool value );
		JsonBoolean( JsonBoolean const& other );
		JsonBoolean & operator=( JsonBoolean const& other );

		bool const& getValue( void ) const;
	private:
		JsonBoolean( void );
		bool _data;
	};

	class JsonNull : public JsonToken
	{
	public:
		~JsonNull();
		JsonNull( void );
		JsonNull( JsonNull const& other );
		JsonNull & operator=( JsonNull const& other );

	private:
	};

	class JsonArray : public JsonToken
	{
	public:
		~JsonArray();
		JsonArray( void );
		JsonArray( std::vector<JsonToken*> const& data );
		JsonArray( JsonArray const& other );
		JsonArray & operator=( JsonArray const& other );

		std::vector<JsonToken*> data;
	private:
	};

	/* json_parse */
	int jsonParseOpenBracket	( t_jparser_info & info );
	int jsonParseCloseBracket	( t_jparser_info & info );
	int jsonParseString			( t_jparser_info & info );
	int jsonParseColon			( t_jparser_info & info );
	int jsonParseComma			( t_jparser_info & info );
	int jsonParseInteger		( t_jparser_info & info );
	int jsonParseBoolean		( t_jparser_info & info );
	int jsonParseNull			( t_jparser_info & info );
	int jsonParseArray			( t_jparser_info & info );

	/* utils */
	int jsonErr( t_jparser_info const& info, char const* err );
	char * jsonReadFile( char const* filepath );

}
