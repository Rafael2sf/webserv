#include "Iterator.hpp"
#include "Node.hpp"

namespace JSON
{
	Iterator::Iterator( void )
	{}

	Iterator::Iterator( Node * p )
	: node(p)
	{}

	Iterator::Iterator( Iterator const& other )
	: node(other.node)
	{
		*this = other;
	}

	Iterator &
	Iterator::operator=( Iterator const& rhs )
	{
		node = rhs.node;
		return *this;
	}

	Node &
	Iterator::operator*( void )
	{
		return *node;
	}

	Node *
	Iterator::operator->( void )
	{
		return node;
	}

	Iterator	&
	Iterator::operator++( void )
	{
		Object *	obj;
		Array *		arr;

		switch (node->type())
		{
			case array:
				arr = dynamic_cast<Array *>(node);
				if (arr->impl.size() == 0)
					node = _findNextFromParent(node->getParent());
				else
					node =  (*arr->impl.begin());
				break ;
			case object:
				obj = dynamic_cast<Object *>(node);
				if (obj->impl.size() == 0)
					node = _findNextFromParent(node->getParent());
				else
					node = (*obj->impl.begin());
				break ;
			default:
				node = _findNextFromParent(node->getParent());
				break ;
		}
		return *this;
	}

	Iterator 
	Iterator::operator++( int )
	{
		Iterator	it(*this);

		++(*this);
		return (it);
	}

	Iterator	&
	Iterator::operator--( void )
	{
		return *this;
	}

	Iterator
	Iterator::operator--( int )
	{
		Iterator	it(*this);

		--(*this);
		return (it);
	}

	Node * Iterator::_findNextFromParent( Node * parent )
	{
		Object * tmp;
		Array * arr;
		std::multiset<Node *>::iterator it;
		std::vector<Node *>::iterator it2;

		if (!parent)
			return node;
		if (parent->type() == array)
		{
			arr = dynamic_cast<Array *>(parent);
			it2 = std::find(arr->impl.begin(), arr->impl.end(), node);
			if (it2 == arr->impl.end() || it2 == --arr->impl.end())
			{
				node = parent;
				node = _findNextFromParent(node->getParent());
			}
			else
				node = *(++it2);
		}
		else
		{
			tmp = dynamic_cast<Object *>(parent);
			it = tmp->impl.find(node);
			if (it == tmp->impl.end() || it == --tmp->impl.end())
			{
				node = parent;
				node = _findNextFromParent(node->getParent());
			}
			else
				node = *++it;
		}
		return node;
	}

	void Iterator::skip( void )
	{
		if (!node->getParent())
			return ;
		_findNextFromParent(node->getParent());
	}

	bool Iterator::operator==( Iterator const&  rhs ) const
	{
		return node == rhs.node;
	}

	bool Iterator::operator!=( Iterator const&  rhs ) const
	{
		return node != rhs.node;
	}

	ConstIterator::ConstIterator( void )
	{}

	ConstIterator::ConstIterator( Node const* p )
	{
		node = const_cast<Node *>(p);
	}

	ConstIterator::ConstIterator( Iterator const& other )
	: node(other.node)
	{
		this->node = other.node;
	}

	ConstIterator::ConstIterator( ConstIterator const& other )
	: node(other.node)
	{
		*this = other;
	}

	ConstIterator &
	ConstIterator::operator=( ConstIterator const& rhs )
	{
		node = rhs.node;
		return *this;
	}

	Node const&
	ConstIterator::operator*( void ) const
	{
		return *node;
	}

	Node const*
	ConstIterator::operator->( void ) const
	{
		return node;
	}

	ConstIterator	&
	ConstIterator::operator++( void )
	{
		Object *	obj;
		Array *		arr;

		switch (node->type())
		{
			case array:
				arr = dynamic_cast<Array *>(node);
				if (arr->impl.size() == 0)
					node = _findNextFromParent(node->getParent());
				else
					node =  (*arr->impl.begin());
				break ;
			case object:
				obj = dynamic_cast<Object *>(node);
				if (obj->impl.size() == 0)
					node = _findNextFromParent(node->getParent());
				else
					node = (*obj->impl.begin());
				break ;
			default:
				node = _findNextFromParent(node->getParent());
				break ;
		}
		return *this;
	}

	ConstIterator 
	ConstIterator::operator++( int )
	{
		ConstIterator	it(*this);

		++(*this);
		return (it);
	}

	ConstIterator	&
	ConstIterator::operator--( void )
	{
		return *this;
	}

	ConstIterator
	ConstIterator::operator--( int )
	{
		ConstIterator	it(*this);

		--(*this);
		return (it);
	}

	Node * ConstIterator::_findNextFromParent( Node * parent )
	{
		Object * tmp;
		Array * arr;
		std::multiset<Node *>::iterator it;
		std::vector<Node *>::iterator it2;

		if (!parent)
			return node;
		if (parent->type() == array)
		{
			arr = dynamic_cast<Array *>(parent);
			it2 = std::find(arr->impl.begin(), arr->impl.end(), node);
			if (it2 == arr->impl.end() || it2 == --arr->impl.end())
			{
				node = parent;
				node = _findNextFromParent(node->getParent());
			}
			else
				node = *(++it2);
		}
		else
		{
			tmp = dynamic_cast<Object *>(parent);
			it = tmp->impl.find(node);
			if (it == tmp->impl.end() || it == --tmp->impl.end())
			{
				node = parent;
				node = _findNextFromParent(node->getParent());
			}
			else
				node = *++it;
		}
		return node;
	}

	void ConstIterator::skip( void )
	{
		if (!node->getParent())
			return ;
		_findNextFromParent(node->getParent());
	}

	bool ConstIterator::operator==( ConstIterator const&  rhs ) const
	{
		return node == rhs.node;
	}

	bool ConstIterator::operator!=( ConstIterator const&  rhs ) const
	{
		return node != rhs.node;
	}
}
