#pragma once

#include <set>
#include <vector>
#include <string>
#include <typeinfo>
#include "Iterator.hpp"

namespace JSON
{
	typedef enum e_node
	{
		none	=	0x00,
		object	=	0x01,
		string	=	0x02,
		integer	=	0x04,
		boolean	=	0x08,
		null	=	0x10,
		array	=	0x20,
		point	=	0x40
	} t_node;

	class Node
	{
	public:
		typedef Iterator iterator;
		typedef ConstIterator const_iterator;

		virtual ~Node();
		Node(void);
		Node(Node const &other);
		Node &operator=(Node const &other);
		Node & operator[](char const* key);

		Iterator begin( void );
		const_iterator begin( void ) const;
		Iterator end( void );
		const_iterator end( void ) const;

		std::string const &getProperty(void) const;
		void setProperty(std::string const &);
		Node *getParent(void) const;
		void setParent(Node *token);
		int type(void) const;
		template <class T> T as( void ) const
		{
			throw std::bad_cast();
		}
		Node * find_first( std::string const& key, size_t max_depth = 1) const;
		Node * search(int depth, ...) const;

	protected:
		std::string _property;
		Node 		*_parent;
		t_node		_type;
	};

	class String : public Node
	{
	public:
		~String();
		String(void);
		String(std::string const& val);
		String(String const &other);
		String &operator=(String const &other);

		std::string impl;
	private:
	};

	class Object : public Node
	{
	public:
		~Object();
		Object(void);
		Object(Object const &other);
		Object &operator=(Object const &other);

		std::multiset<Node *> impl;
	private:
	};

	class Integer : public Node
	{
	public:
		~Integer();
		Integer(void);
		Integer(int value);
		Integer(Integer const &other);
		Integer &operator=(Integer const &other);

		int impl;
	private:
	};

	class Boolean : public Node
	{
	public:
		~Boolean();
		Boolean(void);
		Boolean(bool value);
		Boolean(Boolean const &other);
		Boolean &operator=(Boolean const &other);

		bool impl;
	private:
	};

	class Null : public Node
	{
	public:
		~Null();
		Null(void);
		Null(Null const &other);
		Null &operator=(Null const &other);
	private:
	};

	class Array : public Node
	{
	public:
		~Array();
		Array(void);
		Array(Array const &other);
		Array &operator=(Array const &other);

		std::vector<Node*> impl;
	private:
	};

	class Point : public Node
	{
	public:
		~Point();
		Point(void);
		Point(float value);
		Point(Point const &other);
		Point &operator=(Point const &other);

		float impl;
	private:
	};
}
