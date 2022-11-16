#include "JsonToken.hpp"
#include "Json.hpp"

namespace ft
{
	static void rprint(JsonToken const* p, size_t depth)
	{
		size_t i = 0;

		while (i++ != depth)
			std::cout << " ";
		std::cerr << '[' << p->getProperty() << "] : ";
		if (p->getType() == json_object_type)
		{
			std::cerr << std::endl;
			JsonObject const* tmp = dynamic_cast<JsonObject const*>(p);
			for (std::vector<JsonToken *>::const_iterator it = tmp->data.begin();
				it != tmp->data.end(); it++)
			{rprint(*it, depth + 1);}
			return ;
		}
		if (p->getType() == json_array_type)
		{
			std::cerr << "[";
			JsonArray const* tmp = dynamic_cast<JsonArray const*>(p);
			for (std::vector<JsonToken *>::const_iterator it = tmp->data.begin();
				it != tmp->data.end(); it++)
			{
				switch ((*it)->getType())
				{
					case json_string_type:
						std::cerr << '\"' << Json::getStringOf(*it) << '\"';
						break ;
					case json_boolean_type:
						std::cerr << '^' << Json::getBooleanOf(*it);
						break ;
					case json_integer_type:
						std::cerr << Json::getIntegerOf(*it);
						break ;
					case json_null_type:
						std::cerr << "<null>";
						break ;
					default:
						break ;
				}
				if (it + 1 != tmp->data.end())
					std::cerr << ',';
			}
			std::cerr << ']' << std::endl;
			return ;
		}
		switch (p->getType())
		{
			case json_string_type:
				std::cerr << '\"' << Json::getStringOf(p) << '\"' << std::endl;
				return ;
			case json_boolean_type:
				std::cerr << '^' << Json::getBooleanOf(p) << std::endl;
				return ;
			case json_integer_type:
				std::cerr << Json::getIntegerOf(p) << std::endl;
				return ;
			case json_null_type:
				std::cerr << "<null>" << std::endl;
				return ;
			default:
				return ;
		}
	}

	void JsonToken::print( void ) const
	{
		rprint(this, 0);
	}

	JsonToken * JsonToken::find_first( char const* s )
	{
		std::vector<JsonToken*> tokens;
		if (_type == json_object_type)
		{
			tokens = Json::getObjectOf(this);
			for (std::vector<JsonToken*>::iterator it = tokens.begin();
				it != tokens.end(); it++)
			{
				return (*it)->find_first(s);
			}
		}
		else if (!strcmp(_property, s))
			return (this);
		return 0;
	}

	JsonToken::~JsonToken()
	{
		if (_property)
			delete[] _property;
	}

	JsonToken::JsonToken( void )
	: _property(0), _parent(0), _type(json_none_type)
	{}

	JsonToken::JsonToken( JsonToken const& other )
	{
		*this = other;
	}

	JsonToken & JsonToken::operator=( JsonToken const& other )
	{
		(void) other;
		return *this;
	}

	const char*	JsonToken::getProperty( void ) const
	{
		return _property;
	}

	void JsonToken::setProperty( char * property )
	{
		_property = property;
	}

	JsonToken * JsonToken::getParent( void ) const
	{
		return _parent;
	}

	void JsonToken::setParent( JsonToken * token )
	{
		_parent = token;
	}

	int JsonToken::getType( void ) const
	{
		return _type;
	}

	// json string

	JsonString::~JsonString()
	{
		if (_data)
			delete[] _data;
	}

	JsonString::JsonString( void )
	{
		JsonToken::_type = json_string_type;
	}
	
	JsonString::JsonString( char const* value )
	: _data(const_cast<char*>(value))
	{
		JsonToken::_type = json_string_type;
	}

	JsonString::JsonString( JsonString const& other )
	{
		*this = other;
	}

	JsonString & JsonString::operator=( JsonString const& other )
	{
		(void) other;
		return *this;
	}

	char const* JsonString::getValue( void ) const
	{
		return _data;
	}

	// json object

	JsonObject::~JsonObject()
	{
		for (std::vector<JsonToken *>::const_iterator it = data.begin();
			it != data.end(); it++)
			delete *it;
	}

	JsonObject::JsonObject( void )
	{
		JsonToken::_type = json_object_type;
	}

	JsonObject::JsonObject( JsonObject const& other )
	{
		*this = other;
	}

	JsonObject & JsonObject::operator=( JsonObject const& other )
	{
		(void) other;
		return *this;
	}

	// json integer

	JsonInteger::~JsonInteger()
	{}

	JsonInteger::JsonInteger( void )
	{
		JsonToken::_type = json_integer_type;
	}

	JsonInteger::JsonInteger( int value )
	: _data(value)
	{
		JsonToken::_type = json_integer_type;
	}

	JsonInteger::JsonInteger( JsonInteger const& other )
	{
		*this = other;
	}

	JsonInteger & JsonInteger::operator=( JsonInteger const& other )
	{
		(void) other;
		return *this;
	}

	int const& JsonInteger::getValue( void ) const
	{
		return _data;
	}

	// json bool

	JsonBoolean::~JsonBoolean()
	{}

	JsonBoolean::JsonBoolean( void )
	{
		JsonToken::_type = json_boolean_type;
	}

	JsonBoolean::JsonBoolean( bool value )
	: _data(value)
	{
		JsonToken::_type = json_boolean_type;
	}

	JsonBoolean::JsonBoolean( JsonBoolean const& other )
	{
		*this = other;
	}

	JsonBoolean & JsonBoolean::operator=( JsonBoolean const& other )
	{
		(void) other;
		return *this;
	}

	bool const& JsonBoolean::getValue( void ) const
	{
		return _data;
	}

	// json null

	JsonNull::~JsonNull()
	{}

	JsonNull::JsonNull( void )
	{
		JsonToken::_type = json_null_type;
	}

	JsonNull::JsonNull( JsonNull const& other )
	{
		*this = other;
	}

	JsonNull & JsonNull::operator=( JsonNull const& other )
	{
		(void) other;
		return *this;
	}

	// jsonArray

	JsonArray::~JsonArray()
	{
		for (std::vector<JsonToken *>::const_iterator it = data.begin();
			it != data.end(); it++)
			delete *it;
	}

	JsonArray::JsonArray( void )
	{
		JsonToken::_type = json_array_type;
	}

	JsonArray::JsonArray( std::vector<JsonToken*> const& value )
	: data(value)
	{
		JsonToken::_type = json_array_type;
	}

	JsonArray::JsonArray( JsonArray const& other )
	{
		*this = other;
	}

	JsonArray & JsonArray::operator=( JsonArray const& other )
	{
		(void) other;
		return *this;
	}
}
