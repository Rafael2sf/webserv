#include <iostream>
#include <cstdarg>
#include "Node.hpp"
#include "Iterator.hpp"
//#include "Json.hpp"

namespace JSON
{
	template <>
	int Node::as<int>( void ) const
	{
		return dynamic_cast<Integer const&>(*this).impl;
	}

	template <>
	std::string const& Node::as<std::string const&>( void ) const
	{
		return dynamic_cast<String const&>(*this).impl;
	}

	template <>
	bool Node::as<bool>( void ) const
	{
		return dynamic_cast<Boolean const&>(*this).impl;
	}

	static Node * find_recursive(Node * root, std::string const& key, size_t max_depth)
	{
		Object const*	obj;
		Node * 			x = 0;

		if (max_depth == __SIZE_MAX__)
			return 0;
		if (root->getProperty() == key)
			return (root);
		if (root->type() == object)
		{
			obj = dynamic_cast<Object const*>(root);
			if (!obj)
				return 0;
			for (std::set<Node *>::const_iterator it = obj->impl.begin();
				it != obj->impl.end(); it++)
			{
				x = find_recursive(*it, key, max_depth - 1);
				if (x)
					return x ;
			}
		}
		return x;
	}

	Node * Node::find_first( std::string const& key, size_t max_depth) const
	{
		Object const*	obj;
		Node * 			x = 0;

		if (max_depth == __SIZE_MAX__ || _type != object)
			return 0;
		if (_type == object)
		{
			obj = dynamic_cast<Object const*>(this);
			if (!obj)
				return 0;
			for (std::set<Node*>::const_iterator it = obj->impl.begin();
				it != obj->impl.end(); it++)
			{
				x = find_recursive(*it, key, max_depth - 1);
				if (x)
					return x ;
			}
		}
		return 0;
	}

	Node & Node::operator[](char const* key)
	{
		Node * res = find_first(key);
		if (!res)
			throw std::invalid_argument("no such element");
		return *res;
	}

	Node * Node::search(int depth, ...) const
	{
		va_list	va;
		Node * tmp;

		va_start(va, depth);
		while (depth--)
		{
			tmp = this->find_first(va_arg(va, char const*) , 1);
			if (!tmp)
				return NULL;
		}
		va_end(va);
		return tmp;
	}

	Iterator Node::begin( void )
	{
		if (_type == array || _type == object)
			return ++Iterator(this);
		else
			return Iterator(this);
	}

	ConstIterator Node::begin( void ) const
	{
		if (_type == array || _type == object)
			return ++ConstIterator(this);
		else
			return ConstIterator(this);
	}

	Iterator Node::end( void )
	{
		Iterator it;

		if (getParent())
		{
			if (_type == array || _type == object)
			{
				it = Iterator(this);
				it.skip();
				return it;
			}
			it = this;
			return ++it;
		}
		return Iterator(this);
	}

	ConstIterator Node::end( void ) const
	{
		ConstIterator it;

		if (getParent())
		{
			if (_type == array || _type == object)
			{
				it = ConstIterator(this);
				it.skip();
				return it;
			}
			it = this;
			return ++it;
		}
		return ConstIterator(this);
	}

	Node::~Node()
	{}

	Node::Node( void )
	: _property(), _parent(0), _type(none)
	{}

	Node::Node( Node const& other )
	{
		*this = other;
	}

	Node & Node::operator=( Node const& other )
	{
		_property = other._property;
		return *this;
	}

	std::string const& Node::getProperty( void ) const
	{
		return _property;
	}

	void Node::setProperty( std::string const& property )
	{
		_property = property;
	}

	Node * Node::getParent( void ) const
	{
		return _parent;
	}

	void Node::setParent( Node * token )
	{
		_parent = token;
	}

	int Node::type( void ) const
	{
		return _type;
	}

	// json string

	String::~String()
	{}

	String::String( void )
	{
		Node::_type = string;
	}
	
	String::String( std::string const& value )
	{
		Node::_type = string;
		impl = value;
	}

	String::String( String const& other )
	: Node()
	{
		Node::_type = string;
		*this = other;
	}

	String & String::operator=( String const& other )
	{
		if (this != &other)
			impl = other.impl;
		return *this;
	}

	// std::string const& String::getValue( void ) const
	// {
	// 	return *this;
	// }

	// json object

	Object::~Object()
	{
		for (std::set<Node *>::iterator it = impl.begin();
			it != impl.end(); it++)
			delete *it;
		impl.clear();
	}

	Object::Object( void )
	{
		Node::_type = object;
	}

	Object::Object( Object const& other )
	: Node()
	{
		Node::_type = object;
		*this = other;
	}

	Object & Object::operator=( Object const& other )
	{
		if (this != &other)
			impl = other.impl;
		return *this;
	}

	// json integer

	Integer::~Integer()
	{}

	Integer::Integer( void )
	{
		Node::_type = integer;
	}

	Integer::Integer( int value )
	: impl(value)
	{
		Node::_type = integer;
	}

	Integer::Integer( Integer const& other )
	: Node()
	{
		Node::_type = integer;
		*this = other;
	}

	Integer & Integer::operator=( Integer const& other )
	{
		impl = other.impl;
		return *this;
	}

	// int const& Integer::getValue( void ) const
	// {
	// 	return _data;
	// }

	// json bool

	Boolean::~Boolean()
	{}

	Boolean::Boolean( void )
	{
		Node::_type = boolean;
	}

	Boolean::Boolean( bool value )
	: impl(value)
	{
		Node::_type = boolean;
	}

	Boolean::Boolean( Boolean const& other )
	: Node()
	{
		Node::_type = boolean;
		*this = other;
	}

	Boolean & Boolean::operator=( Boolean const& other )
	{
		impl = other.impl;
		return *this;
	}

	// bool const& Boolean::getValue( void ) const
	// {
	// 	return _data;
	// }

	// json null

	Null::~Null()
	{}

	Null::Null( void )
	{
		Node::_type = null;
	}

	Null::Null( Null const& other )
	: Node()
	{
		Node::_type = null;
		*this = other;
	}

	Null & Null::operator=( Null const& other )
	{
		(void) other;
		return *this;
	}

	// Array

	Array::~Array()
	{
		for (std::vector<Node *>::iterator it = impl.begin();
			it != impl.end(); it++)
			delete *it;
		impl.clear();
	}

	Array::Array( void )
	{
		Node::_type = array;
	}

	Array::Array( Array const& other )
	: Node()
	{
		Node::_type = array;
		*this = other;
	}

	Array & Array::operator=( Array const& other )
	{
		if (this != &other)
			impl = other.impl;
		return *this;
	}
}
