#include "Json.hpp"
#include "JsonToken.hpp"
#include <stdexcept>

namespace ft
{
	Json::~Json()
	{
		for (std::list<JsonToken *>::const_iterator it = tokens.begin();
			it != tokens.end(); it++)
		{
			delete *it;
		}
	}

	Json::Json( void )
	{}

	Json::Json( Json const& other )
	{
		*this = other;
	}

	Json & Json::operator=( Json const& other )
	{
		(void) other;
		return *this;
	}

	char const*
	Json::getStringOf( JsonToken const* t )
	{
		return dynamic_cast<JsonString const*>(t)->getValue();
	}

	bool const&
	Json::getBooleanOf( JsonToken const* t )
	{
		return dynamic_cast<JsonBoolean const*>(t)->getValue();
	}
		
	int const&
	Json::getIntegerOf( JsonToken const* t )
	{
		return dynamic_cast<JsonInteger const*>(t)->getValue();
	}

	std::vector<JsonToken*> const&
	Json::getObjectOf( JsonToken const* t )
	{
		return dynamic_cast<JsonObject const*>(t)->data;
	}

	void Json::cout( void ) const
	{
		for (std::list<JsonToken *>::const_iterator it = tokens.begin();
			it != tokens.end(); it++)
			(*it)->print();
	}
}
