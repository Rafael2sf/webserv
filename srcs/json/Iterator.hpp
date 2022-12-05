#pragma once

#include <iterator>
#include <iostream>
#include <algorithm>

namespace JSON
{
	class Node;

	class Iterator: public std::iterator_traits<Node *>
	{
	public:
		typedef std::bidirectional_iterator_tag iterator_category;
		Node * node;

		Iterator( void );
		Iterator( Node * __p );
		Iterator( Iterator const& __other );

		Iterator &	operator=( Iterator const& rhs );
		Node &		operator*( void );
		Node *		operator->( void );
		Iterator &	operator++( void );
		Iterator 	operator++( int );
		Iterator &	operator--( void );
		Iterator	operator--( int );
		bool		operator==( Iterator const&  rhs ) const;
		bool		operator!=( Iterator const&  rhs ) const;
		void		skip( void );
	
	private:
		Node * _findNextFromParent( Node * parent );
	};

	class constIterator: public std::iterator_traits<Node *>
	{
	public:
		typedef std::bidirectional_iterator_tag iterator_category;
		Node * node;

		constIterator( void );
		constIterator( Node * __p );
		constIterator( Iterator const& other );
		constIterator( constIterator const& __other );

		constIterator &	operator=( constIterator const& rhs );
		Node const&		operator*( void ) const;
		Node const*		operator->( void ) const;
		constIterator &	operator++( void );
		constIterator 	operator++( int );
		constIterator &	operator--( void );
		constIterator	operator--( int );
		bool		operator==( constIterator const&  rhs ) const;
		bool		operator!=( constIterator const&  rhs ) const;
		void		skip( void );
	
	private:
		Node * _findNextFromParent( Node * parent );
	};
}
