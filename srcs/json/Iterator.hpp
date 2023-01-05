#pragma once

#include <iterator>
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

	class ConstIterator: public std::iterator_traits<Node *>
	{
	public:
		typedef std::bidirectional_iterator_tag iterator_category;
		Node * node;

		ConstIterator( void );
		ConstIterator( Node const* __p );
		ConstIterator( Iterator const& other );
		ConstIterator( ConstIterator const& __other );

		ConstIterator &	operator=( ConstIterator const& rhs );
		Node const&		operator*( void ) const;
		Node const*		operator->( void ) const;
		ConstIterator &	operator++( void );
		ConstIterator 	operator++( int );
		ConstIterator &	operator--( void );
		ConstIterator	operator--( int );
		bool		operator==( ConstIterator const&  rhs ) const;
		bool		operator!=( ConstIterator const&  rhs ) const;
		void		skip( void );
	
	private:
		Node * _findNextFromParent( Node * parent );
	};
}
